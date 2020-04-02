#include <wasm.hpp>
#include <asset.hpp>
#include <table.hpp>
#include <string>
using namespace wasm;
using namespace std;


//static constexpr int64_t  one_day = 1*24*60*60;
static constexpr int64_t  one_day = 1;

struct file_t{
    //std::vector<approval> requested_approvals;
    uint64_t          deadline; // in days
    name              action;
    std::vector<char> params;//use rpcapi jsontobinwasm to serialize
};

CONTRACT msig : public contract {
   public:
      using contract::contract;
      ACTION file1(uint64_t p1, uint64_t p2);
      ACTION file2(string   p);
      
      ACTION propose(regid proposer, name proposal_name, file_t file);
      ACTION approve(regid proposer, name proposal_name, regid approver);
      ACTION exec( regid proposer, name proposal_name, regid executer );
      // ACTION unapprove(name proposer, name proposal_name, name unapprover);

      ACTION setthreshold(name file, uint64_t threshold);
      ACTION setadmin(regid admin,    uint64_t weight);
  private:
      TABLE proposal_t {
          name   proposal_name;
          file_t file; 
          name  primary_key()const { return proposal_name; }
      };
      // struct approval {
      //     name     account;
      //     uint64_t weight;
      //     uint64_t time;
      // };
      TABLE approvals_info_t {
          name     proposal_name;
          uint64_t threshold; 
          uint64_t provided;
          uint64_t deadline;
          //std::vector<approval> requested_approvals;
          //std::vector<approval> provided_approvals;
          bool     closed;
          name  primary_key()const { return proposal_name; }
      };

      TABLE authority_t {
          name     file;
          uint64_t threshold;
          name  primary_key()const { return file; }
      };

      TABLE admin_t {
          regid     account;
          uint64_t weight;
          regid  primary_key()const { return account; }
      };

      typedef wasm::table< "proposals"_n, proposal_t, name > proposals;  
      typedef wasm::table< "approvals2"_n, approvals_info_t, name > approvals2; 
      typedef wasm::table< "authorities"_n, authority_t, name > authorities; 
      typedef wasm::table< "admins"_n, admin_t, regid > admins; 
};
