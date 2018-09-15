#!/bin/bash

cleos push action token create '["1000000 TTE", "TestTokenEOS", "admin"]' -p token@active
cleos push action token issue '["admin", "500000 TTE", "500000 TTE"]' -p token@active

cleos push action token transfer '["admin", "user.a", "100 TTE"]' -p admin@active
cleos push action token transferfrz '["user.a", "101 TTE"]' -p admin@active

cleos push action token transfer '["admin", "user.b", "200 TTE"]' -p admin@active
cleos push action token transferfrz '["user.b", "201 TTE"]' -p admin@active

cleos push action token transfer '["admin", "user.c", "300 TTE"]' -p admin@active
cleos push action token transferfrz '["user.c", "301 TTE"]' -p admin@active

cleos push action token show '["admin"]' -p token@active
cleos push action token show '["user.a"]' -p token@active
cleos push action token show '["user.b"]' -p token@active
cleos push action token show '["user.c"]' -p token@active
cleos push action token show '["user.d"]' -p token@active

read -p "Press enter to continue..."
	
cleos push action token activate '["user.a", "51 TTE"]' -p admin@active
cleos push action token activate '["user.b", "51 TTE"]' -p admin@active
cleos push action token activate '["user.c", "51 TTE"]' -p admin@active

cleos push action token show '["admin"]' -p token@active
cleos push action token show '["user.a"]' -p token@active
cleos push action token show '["user.b"]' -p token@active
cleos push action token show '["user.c"]' -p token@active
cleos push action token show '["user.d"]' -p token@active

read -p "Press enter to continue..."

cleos push action token transfer '["user.a", "user.d", "51 TTE"]' -p user.a@active
cleos push action token transferfrz '["user.d", "50 TTE"]' -p user.a@active
cleos push action token transfer '["user.b", "user.d", "151 TTE"]' -p user.b@active
cleos push action token transferfrz '["user.d", "150 TTE"]' -p user.b@active
cleos push action token transfer '["user.c", "user.d", "251 TTE"]' -p user.c@active
cleos push action token transferfrz '["user.d", "250 TTE"]' -p user.c@active

cleos push action token show '["admin"]' -p token@active
cleos push action token show '["user.a"]' -p token@active
cleos push action token show '["user.b"]' -p token@active
cleos push action token show '["user.c"]' -p token@active
cleos push action token show '["user.d"]' -p token@active
