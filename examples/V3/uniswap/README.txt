uniswap
-------------

 - How to Build -
   - cd to 'build' directory
   - run the command 'cmake ..'
   - run the command 'make'

 - After build -
   - The built smart contract is under the 'uniswap' directory in the 'build' directory
   - You can then do a 'set contract' action and point in to the './build/uniswap' directory

 - Additions to CMake should be done to the CMakeLists.txt in the './src' directory and not in the top level CMakeLists.txt

 - Sample -
delop contract
./coind -datadir= submitsetcodetx 0-2 /token.wasm /token.abi 113901-2
./coind -datadir= submitsetcodetx 0-2 /mpas.wasm /mpas.abi 118093-2
./coind -datadir= submitsetcodetx 0-2 /uniswap.wasm /uniswap.abi 118109-2  

create market    
./coind -datadir= submittx 0-2 118109-2 create '["113901-2","113901-2","8,BTC","8,ETH","118093-2"]'
./coind -datadir= submittx 0-2 118093-2 set_owner '["118109-2"]'

mint
./coind -datadir= submittx 0-2 113901-2 transfer '["0-2","118109-2","10000.00000000 BTC","mint:BTC-ETH:1:1"]'
./coind -datadir= submittx 0-2 113901-2 transfer '["0-2","118109-2","10000.00000000 ETH","mint:BTC-ETH:2:1"]'

burn
./coind -datadir= submittx 0-2 118109-2 burn '["BTC-ETH","0-2"]'

swap
./coind -datadir= submittx 0-2 113901-2 transfer '["0-2","118109-2","10000.00000000 BTC","swap:BTC-ETH:8000.00000000 ETH:0-2"]'