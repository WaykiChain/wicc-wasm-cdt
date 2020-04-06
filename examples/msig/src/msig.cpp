#include <msig.hpp>
#include <system.hpp>
#include <strings.hpp>

using namespace wasm;


ACTION msig::propose(regid proposer, name proposal_name, file_t file){
    require_auth(proposer);

    proposal_t proposal_object;
    proposals proposals_table(get_self(), proposer.value);
    check(!proposals_table.get(proposal_object, proposal_name), "proposal already exists");
    proposals_table.emplace(get_self(), [&](auto &s) {
        s.proposal_name = proposal_name;
        s.file          = file;
    });

    authority_t authority_object;
    authorities authorities_table(get_self(), get_self().value);
    check(authorities_table.get(authority_object, file.action), "authority doesn't exists");
    check(authority_object.threshold > 0, "authority threshold must > 0");

    approvals_info_t approvals_info_object;
    approvals2 approvals2_table(get_self(), proposer.value);
    check(!approvals2_table.get(approvals_info_object, proposal_name), "approvals infos already exists");
    approvals2_table.emplace(get_self(), [&](auto &s) {
        s.proposal_name = proposal_name;
        s.threshold     = authority_object.threshold;
        s.provided      = 0;
        s.deadline      = current_block_time() + file.deadline * one_day;
        s.closed        = false;
    });

}

ACTION msig::approve(regid proposer, name proposal_name, regid approver){
    require_auth(approver);

    admin_t admin_object;
    admins admins_table(get_self(), get_self().value);
    check(admins_table.get(admin_object, approver), "approver doesn't exists");

    approvals_info_t approvals_info_object;
    approvals2 approvals2_table(get_self(), proposer.value);
    check(approvals2_table.get(approvals_info_object, proposal_name), "approvals infos doesn't exists");  
    check(approvals_info_object.deadline >= current_block_time(), "out of deadline"); 
    check(!approvals_info_object.closed, "proposal closed");    

    approvals2_table.modify(approvals_info_object, wasm::no_payer, [&](auto &s) {
        s.provided      += admin_object.weight;
    });

}

// ACTION msig::unapprove(name proposer, name proposal_name, name unapprover){
//     require_auth(unapprover);

//     admin_t admin_object;
//     admins admins_table(get_self(), get_self().value);
//     check(!admins_table.get(admin_object, unapprover), "unapprover doesn't exists");

//     approvals_info_t approvals_info_object;
//     approvals2 approvals2_table(get_self(), proposer.value);
//     check(approvals2_table.get(approvals_info_object, proposal_name), "approvals infos doesn't exists"); 
//     approvals2_table.modify(approvals_info_object, wasm::no_payer, [&](auto &s) {
//         s.provided      -= admin_object.weight;
//     });

// }

ACTION msig::exec( regid proposer, name proposal_name, regid executer ){
    require_auth( executer );

    approvals_info_t approvals_info_object;
    approvals2 approvals2_table(get_self(), proposer.value);
    check(approvals2_table.get(approvals_info_object, proposal_name), "approvals infos doesn't exists");   
    check(!approvals_info_object.closed, "proposal closed");   
    check(approvals_info_object.threshold <= approvals_info_object.provided, "didn't get enough approvals");

    approvals2_table.modify(approvals_info_object, wasm::no_payer, [&](auto &s) {    
        s.closed = true;

        proposal_t proposal_object;
        proposals proposals_table(get_self(), proposer.value);
        check(proposals_table.get(proposal_object, proposal_name),"proposal doesn't exists");

        std::vector<char> param_bytes = from_hex(proposal_object.file.params);
        if(proposal_object.file.action == name("file1")){
            std:tuple<uint64_t,uint64_t> params = unpack<std::tuple<uint64_t,uint64_t>>(param_bytes);
            file1(std::get<0>(params), std::get<1>(params));
        } else if(proposal_object.file.action == name("file2")){
            string params = unpack<string>(param_bytes);
            file2(params);
        }
    });

}

ACTION msig::setthreshold(name file, uint64_t threshold){
    require_auth(get_self());

    authority_t authority_object;
    authorities authorities_table(get_self(), get_self().value);
    if(!authorities_table.get(authority_object, file)){
        authorities_table.emplace(get_self(), [&](auto &s) {
            s.file      = file;
            s.threshold = threshold;   
        });
    } else {
        authorities_table.modify(authority_object, wasm::no_payer, [&](auto &s) {
            s.threshold = threshold;
        }); 
    }
}

ACTION msig::setadmin(regid admin, uint64_t weight){
    require_auth(get_self());

    admin_t admin_object;
    admins admins_table(get_self(), get_self().value);
    if(!admins_table.get(admin_object, admin)){
        admins_table.emplace(get_self(), [&](auto &s) {
            s.account = admin;
            s.weight  = weight;   
        });
    } else {
        admins_table.modify(admin_object, wasm::no_payer, [&](auto &s) {
            s.weight = weight;
        }); 
    }
}


ACTION msig::file1(uint64_t p1, uint64_t p2) {
    print(p1);
    print(p2);
}

ACTION msig::file2(string p) {
    print(p);
}

extern "C" {
void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    switch (action) {
        case wasm::name("propose").value:
            wasm::execute_action(wasm::regid(receiver), wasm::regid(code), &msig::propose);
            break;
        case wasm::name("approve").value:
            wasm::execute_action(wasm::regid(receiver), wasm::regid(code), &msig::approve);
            break;
        // case wasm::name("unapprove").value:
        //     wasm::execute_action(wasm::name(receiver), wasm::name(code), &msig::unapprove);
        //     break;
        case wasm::name("exec").value:
            wasm::execute_action(wasm::regid(receiver), wasm::regid(code), &msig::exec);
            break;
        case wasm::name("setthreshold").value:
            wasm::execute_action(wasm::regid(receiver), wasm::regid(code), &msig::setthreshold);
            break;
        case wasm::name("setadmin").value:
            wasm::execute_action(wasm::regid(receiver), wasm::regid(code), &msig::setadmin);
            break;
        default:
            check(false, "action does not exist");
            break;
    }
}
}