#include "territory.h"
#include "flight_ownership.h"
#include <limits>
#include <stdexcept>

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error("Failed: " #condition); } while (false)

static void add(DataBase& db, const std::string& name, const std::string& parent = "",
                double lo = 0, double hi = 100, const std::string& dim = "Overworld") {
    CHECK(db.addTerritory(name,lo,0,lo,hi,100,hi,lo,1,lo,"Alice","Joanne","Bob",dim,parent,
                          0,0,0,0,0,0,0,0) == SQLITE_OK);
}

static auto rows(DataBase& db, const std::string& name = "") {
    std::vector<std::map<std::string,std::string>> result;
    CHECK(db.readTerritories(result,name) == SQLITE_OK);
    return result;
}

struct TestTerritory : Territory {
    const endstone::PluginDescription& getDescription() const override {
        throw std::logic_error("Host metadata is not used by contract tests");
    }
};

int main() {
    DataBase db(":memory:");
    CHECK(db.init_database() == SQLITE_OK);
    TestTerritory plugin;
    plugin.config_max_tty_area = 4000000;
    translate lang("missing-test-languages");
    Territory_Action action(db, &plugin, lang);
    add(db,"parent");
    add(db,"child","parent",20,30);
    (void)action.get_all_tty();
    CHECK(!action.listTrueFatherTTY("Ann", {{40,5,40},{50,10,50}}, "Overworld").first);
    CHECK(action.listTrueFatherTTY("Joanne", {{40,5,40},{50,10,50}}, "Overworld").first);
    CHECK(!Territory_Action::read_territory_by_name("parent")->cached_all_members.contains("Bo"));
    CHECK(Territory_Action::read_territory_by_name("parent")->cached_all_members.contains("Bob"));

    // Benign punctuation must round-trip through the production lookup and rename.
    CHECK(action.rename_player_tty("parent", "Alice's place; west").first);
    CHECK(rows(db,"Alice's place; west").size() == 1);
    CHECK(rows(db,"missing' punctuation").empty());
    CHECK(Territory_Action::read_territory_by_name("child")->father_tty == "Alice's place; west");
    CHECK(action.list_in_tty({25,5,25},"Overworld")->front().name == "child");
    CHECK(action.list_in_tty({50,5,50},"Overworld")->front().name == "Alice's place; west");
    CHECK(!action.list_in_tty({50,5,50},"Nether"));

    // Failure after parent rename must roll back both persistence and published snapshot.
    CHECK(db.executeSQL("CREATE TRIGGER fail_child BEFORE UPDATE OF father_tty ON territories BEGIN SELECT RAISE(ABORT,'injected'); END") == SQLITE_OK);
    const auto before_rename = rows(db);
    CHECK(!action.rename_player_tty("Alice's place; west", "replacement").first);
    CHECK(rows(db) == before_rename);
    CHECK(Territory_Action::read_territory_by_name("Alice's place; west"));
    CHECK(db.executeSQL("DROP TRIGGER fail_child") == SQLITE_OK);

    // Mid-resize failure: the earlier coordinate writes and teleport stay unchanged.
    CHECK(db.executeSQL("CREATE TRIGGER fail_resize BEFORE UPDATE OF pos2_x ON territories BEGIN SELECT RAISE(ABORT,'injected'); END") == SQLITE_OK);
    const auto before_resize = rows(db);
    auto original = *Territory_Action::read_territory_by_name("child");
    CHECK(!action.resize_territory({21,0,21},{29,90,29},original,{25,1,25}).first);
    CHECK(rows(db) == before_resize);
    CHECK(Territory_Action::read_territory_by_name("child")->pos1 == original.pos1);
    CHECK(db.executeSQL("DROP TRIGGER fail_resize") == SQLITE_OK);
    CHECK(action.resize_territory({21,0,21},{29,90,29},original,{25,1,25}).first);
    CHECK(std::get<0>(Territory_Action::read_territory_by_name("child")->pos1) == 21);
    CHECK(action.list_in_tty({20,5,20},"Overworld")->front().name == "Alice's place; west");

    // Refused delete cannot create a refund intent or detach child claims.
    CHECK(db.executeSQL("CREATE TRIGGER fail_delete BEFORE DELETE ON territories BEGIN SELECT RAISE(ABORT,'injected'); END") == SQLITE_OK);
    std::string payment;
    CHECK(!action.del_Tty_by_name("Alice's place; west", "Alice", 123, &payment));
    CHECK(payment.empty());
    CHECK(Territory_Action::read_territory_by_name("child")->father_tty == "Alice's place; west");
    CHECK(db.executeSQL("DROP TRIGGER fail_delete") == SQLITE_OK);
    CHECK(action.del_Tty_by_name("Alice's place; west", "Alice", 123, &payment));
    CHECK(!payment.empty());
    CHECK(Territory_Action::read_territory_by_name("child")->father_tty.empty());
    CHECK(!action.del_Tty_by_name("Alice's place; west", "Alice", 123, &payment));
    int provider_calls = 0;
    CHECK(!db.applyMoney(payment,[&] { ++provider_calls; return false; }));
    CHECK(!db.applyMoney(payment,[&] { ++provider_calls; return true; }));
    CHECK(provider_calls == 1);
    auto next = db.queueMoney("Alice", 5, "test");
    CHECK(!next.empty());
    CHECK(db.applyMoney(next,[&] { ++provider_calls; return true; }));
    CHECK(!db.applyMoney(next,[&] { ++provider_calls; return true; }));
    CHECK(provider_calls == 2);
    auto uncertain = db.queueMoney("Alice",5,"test unknown provider outcome");
    CHECK(!uncertain.empty());
    try { (void)db.applyMoney(uncertain,[]() -> bool { throw std::runtime_error("unknown"); }); CHECK(false); }
    catch (const std::runtime_error& e) { CHECK(std::string(e.what()) == "unknown"); }
    CHECK(db.queueMoney("Alice",5,"must not repeat").empty());

    // Inclusive block geometry, negative fractions, finite arithmetic and index work.
    CHECK(claim_geometry::area(0,0,0,10) == 11);
    CHECK(claim_geometry::area(-1.2,-1.2,0.2,0.2) == 9);
    CHECK(claim_geometry::area(-30000000,-30000000,30000000,30000000) == 3600000120000001LL);
    CHECK(!claim_geometry::indexable({0,0,0},{1000000,0,0}));
    CHECK(!claim_geometry::indexable({0,0,0},{65535,0,65535}));
    bool rejected = false;
    try { (void)claim_geometry::block(std::numeric_limits<double>::infinity()); } catch (...) { rejected = true; }
    CHECK(rejected);
    CHECK(std::get<0>(Territory_Action::pos_to_tuple("-0.5 1 2")) == -1);
    rejected = false;
    try { (void)Territory_Action::pos_to_tuple("1 2 nope"); } catch (...) { rejected = true; }
    CHECK(rejected);

    // Piston guard remains indexed and dimension-aware, including outside entry.
    CHECK(action.del_Tty_by_name("child"));
    add(db,"protected", "",0,100);
    (void)action.get_all_tty();
    CHECK(!action.canPistonAffect({-1,50,50},"Overworld"));
    CHECK(action.getInstance("")->canPiston({-1,50,50},"Overworld") == false);
    CHECK(!action.canPistonAffect({99,50,50},"Overworld"));
    CHECK(action.canPistonAffect({50,50,50},"Overworld"));
    CHECK(action.canPistonAffect({-20,50,50},"Overworld"));
    CHECK(action.canPistonAffect({-1,50,50},"Nether"));
    CHECK(db.updateValue("territories","if_edge_piston","1","name","protected"));
    (void)action.get_all_tty("protected");
    CHECK(action.canPistonAffect({-1,50,50},"Overworld"));

    // A failed read must not unpublish protection, and failed insertion must not
    // introduce a claim/index entry before durable storage accepts it.
    CHECK(db.executeSQL("ALTER TABLE territories RENAME TO hidden_territories") == SQLITE_OK);
    rejected = false;
    try { (void)action.get_all_tty(); } catch (...) { rejected = true; }
    CHECK(rejected);
    CHECK(action.list_in_tty({50,50,50},"Overworld").has_value());
    CHECK(db.executeSQL("ALTER TABLE hidden_territories RENAME TO territories") == SQLITE_OK);
    CHECK(db.executeSQL("CREATE TRIGGER fail_insert BEFORE INSERT ON territories BEGIN SELECT RAISE(ABORT,'injected'); END") == SQLITE_OK);
    CHECK(!action.create_territory("new owner",{-10.5,0,-10.5},{-2.5,10,-2.5},{-5,1,-5},"Overworld").first);
    CHECK(!action.list_in_tty({-5,1,-5},"Overworld"));
    CHECK(db.executeSQL("DROP TRIGGER fail_insert") == SQLITE_OK);
    CHECK(action.create_territory("new owner",{-10.5,0,-10.5},{-2.5,10,-2.5},{-5,1,-5},"Overworld").first);
    CHECK(std::get<0>(action.list_in_tty({-11,1,-11},"Overworld")->front().source->pos1) == -11);

    FlightOwnership flight;
    CHECK(!flight.reconcile("external",true,true,true));
    CHECK(!flight.reconcile("external",false,true,true));
    CHECK(flight.reconcile("owned",true,false,true) == true);
    CHECK(flight.reconcile("owned",false,true,true) == false);
    CHECK(!flight.reconcile("owned",false,false,true));
    CHECK(flight.reconcile("quit",true,false,true) == true);
    CHECK(flight.remove("quit"));
    CHECK(!flight.reconcile("quit",false,true,true));
    CHECK(flight.reconcile("creative",true,false,true) == true);
    CHECK(!flight.reconcile("creative",false,true,false));
    CHECK(!flight.remove("creative"));
    Territory_Action::clearCache();
    std::cout << "Territory contract tests passed\n";
}
