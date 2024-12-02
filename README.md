# ft_irc
My own IRC server.

## Usage
| command | info |
| ------- | ---- |
| `make` | compile the *ircserv* executable. |
| `make clean` | remove temporary files. |
| `make fclean` | remove temporary and executable. |
| `make re` | force recompile. |

To execute the *server*:
```shell
./ircserv <port> <password>
```

## Supported Commands

| command | syntax | description |
| ------- | ------ | ----------- |
| PING | PING [\<message>] | Ping server. |
| QUIT | QUIT [\<message>] | Leave server. |
| PASS | PASS \<password> | Insert server password (required for authentication). |
| USER | USER \<username> | Set username (required for authentication). |
| NICK | NICK \<nickname> | Set/Change nickname (required for authentication). |
| JOIN | JOIN \<ch1,ch2,...,chN> [\<key1,key2,...,keyN>] | Join/Create one or more channels, optionally with passwords. |
| PART | PART \<channel> [\<message>] | Leave a channel. |
| PRIVMSG | PRIVMSG \<nickname/#channel> \<message> | Send a message to a specific user or to all the users in a specific channel. |
| INVITE | INVITE \<nickname> \<channel> | Invite user to channel. |
| TOPIC | TOPIC \<channel> [\<topic>] | View/Set topic of a channel. |
| KICK | KICK \<channel> \<nickname> [\<message>] | Kick user from a channel. |
| MODE | MODE <channel> [+/-]\<flags> \<args> | Change settings of a channel. <br> - flags: <br> i -> add/remove invite-only mode. <br> t -> add/remove operators-only for TOPIC command. <br> k -> set/remove password. <br> o -> give/take channel operator privileges to user. <br> l -> add/remove user limit. <br> n -> add/remove users-in-channel-only for messages towards channel. |

## Ignored Commands
| command |
| ------- |
| CAP |
| WHO |
| USERHOST |

Note: The usage of any other command will result in an error.
