# ft_irc
My own IRC server.

## Usage
| command | info |
| ------- | ---- |
| `make` | compile the the *server*. |

To execute the *server*:
```shell
./ircserv <port> <password>
```

## Supported Commands

| command | syntax | description |
| ------- | ------ | ----------- |
| PING | PING [\<message>] | Ping server. |
| QUIT | QUIT [\<message>] | Leave server. |
| PASS | PASS \<password> | Insert password (necessary for authentication). |
| USER | USER \<username> | Set username (necessary for authentication). |
| NICK | NICK \<nickname> | Set/Change nickname (necessary for authentication). |
| JOIN | JOIN \<ch1,ch2,...,chN> [\<key1,key2,...,keyN>] | Join/Create a channel. |
| PART | PART \<channel> [\<message>] | Leave a channel. |
| PRIVMSG | PRIVMSG \<nickname/channel> \<message> | Send a message to a specific user or to all the users in a specific channel. |
| INVITE | INVITE \<nickname> \<channel> | Invite user to channel. |
| TOPIC | TOPIC \<channel> [\<topic>] | View/Set topic of a channel. |
| KICK | KICK \<#channel> \<nickname> [\<message>] | Kick user from a channel. |
| MODE | MODE <channel> {[+\|-]\|i\|t\|k\|o\|l} [\<user>] [\<limit>] [\<password>] | Change settings of a channel. |

## Ignored Commands
| command |
| ------- |
| CAP |
| WHO |
| USERHOST |

Note: The usage of any other command will result in an error.
