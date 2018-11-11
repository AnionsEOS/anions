#include <eosiolib/eosio.hpp>

using namespace eosio;

// Smart Contract Name: anions
// Table struct:
//   anionsstruct: multi index table to store the service requests
//     prim_key(uint64): primary key
//     user(name): account name for the user
//     title(string): the title message
//     timestamp(uint64): the store the last update block time
// Public method:
//   isnewuser => to check if the given account name has service request in table or not
// Public actions:
//   update => put the service request into the multi-index table and sign by the given account

// Replace the contract class name when you start your own project
CONTRACT anions : public eosio::contract {
  private:
    bool isnewuser( name user ) {
      // get service requests by using secordary key
      auto req_index = _req.get_index<name("getbyuser")>();
      auto req_iterator = req_index.find(user.value);

      return req_iterator == req_index.end();
    }

    TABLE anionsstruct {
      uint64_t      prim_key;  // primary key
      name          user;      // account name for the user
      std::string   time;      // the time message
      std::string   desc;      // the desc message
      std::string   title;      // the title message
      uint64_t      timestamp; // the store the last update block time

      // primary key
      auto primary_key() const { return prim_key; }
      // secondary key
      // only supports uint64_t, uint128_t, uint256_t, double or long double
      uint64_t get_by_user() const { return user.value; }
    };

    // create a multi-index table and support secondary key
    typedef eosio::multi_index< name("anionsstruct"), anionsstruct,
      indexed_by< name("getbyuser"), const_mem_fun<anionsstruct, uint64_t, &anionsstruct::get_by_user> >
      > anions_table;

    anions_table _anions;

  public:
    using contract::contract;

    // constructor
    anions( name receiver, name code, datastream<const char*> ds ):
                contract( receiver, code, ds ),
                _anions( receiver, receiver.value ) {}

    ACTION update( name user, std::string& anion ) {
      // to sign the action with the given account
      require_auth( user );

      // create new / update service request depends whether the user account exist or not
      if (isnewuser(user)) {
        // insert new service request
        _anions.emplace( _self, [&]( auto& new_user ) {
          new_user.prim_key    = _anions.available_primary_key();
          new_user.user        = user;
          new_user.title        = title;
          new_user.time        = time;
          new_user.desc        = desc;
          new_user.timestamp   = now();
        });
      } else {
        // get object by secordary key
        auto anion_index = _anions.get_index<name("getbyuser")>();
        auto &anion_entry = anion_index.get(user.value);
        // update existing service requests
        _anions.modify( anion_entry, _self, [&]( auto& modified_user ) {
          modified_user.title      = title;
          modified_user.time      = time;
          modified_user.desc      = desc;
          modified_user.timestamp = now();
        });
      }
    }

};

// specify the contract name, and export a public action: update
EOSIO_DISPATCH( anions, (update) )
