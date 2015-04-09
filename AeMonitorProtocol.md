# MONITORS/AE-DAEMON/AE-MANAGER PROTOCOL SPECIFICATION #

Between the 'ae' adaemon and monitors, the `Aeproxy/AeManager`(Android Manager) and 'ae' daemon, the protocol is request/response style and there are 2 class of messages:
  * Control Message
  * Event Message

The same protocol is used to communicate between ae manager (i.e. Android client) and the ae daemon.

_The maximum message length is 1203 bytes.  Any message longer than that will be considered invalid and ae daemon will restart the monitor._

# Control Message #

The control message is defined to be

> ### _**`[:<Text-Header>:]`**_ ###

Where the _**`[:`**_ is the beginning marker and the _**`:]`**_ is the ending marker.

The field _**`<Text-Header>`**_ is defined to be

> ### _**`<version>:<msg-id>:<msg-type>:<name>`**_ ###

Where the  _**:**_ is the sub-field delimiter. Each sub-field is defined in the tables below.

### _**`<version>`**_ - Version ###
| **Value** | **Comment** |
|:----------|:------------|
| 10 | Protocol version string |

### _**`<msg-id>`**_ - Message ID ###
| **Value** | **Comment** |
|:----------|:------------|
| Unique Number | A string, combination of 16 digits, append char '-', append count (1 digit to 6 digits). The 16 digits are from a combination of a high resolution timer that may not corresponds to a real time. The count is the message counter of the monitor process. |

### _**`<msg-type>`**_ - Message type ###
| **Value** | **Comment** |
|:----------|:------------|
| 00 | The HELLO message |
| 11 | The ACK message |
| 22 | The event message |
| 33 | The action message |
| 77 | The OAUTH authentication message |
| 88 | The userid and password authentication message |

### _**`<name>`**_ - Monitor name ###
| **Value** | **Comment** |
|:----------|:------------|
| AE | For ae daemon |
| SM | For socket monitor |
| PM | For proc monitor |
| FM | For file monitor |
| FD | For file descriptor monitor |
| SF | For ae itself monitor |
| AM | For ae manager |

# Event Message #

The event message is defined to be

> ### _**`[:<Text-Header>:<event-id>:<status-op>:<action-list>:<text>:]`**_ ###

Where the _**`[:`**_ is the beginning marker, the _**`:]`**_ is the ending marker, and the _**`:`**_ is the field delimiter.

The field _**`<Text-Header>`**_ is defined under the control message header above. For the rest of fields, they are defined in the tables below.

### _**`<event-id>`**_ - Event associated with the monitor sensors ###
| **Value** | **Comment** |
|:----------|:------------|
| 0001 | The Black issue. Reported by socketmon. |
| 0002 | The White issue. Reported by socketmon. |
| 0003 | Expected process is missing. Reported by procmon. |
| 0004 | File checksum mismatch. Reported by filemon. |
| 0005 | File descriptor changed.  Reported by filedescr |
| etc | Other event codes ... |
| 9999 | initial channel validation event |

### _**`<status-op>`**_ - Status code or opcode of the monitor ###
| **Value** | **Comment** |
|:----------|:------------|
| 00 | GREEN |
| 01 | ORANGE |
| 11 | RED |

### _**`<action-list>`**_ - An array of possible action list for that event defined by _**`<event-id>`**_ ###
| **Value** | **Comment** |
|:----------|:------------|
| A0 | Ignore |
| A1 | Log The Message|


### _**`<text>`**_ - A text message to be transmitted ###
| **Value** | **Comment** |
|:----------|:------------|
| ASCII String | Text message. Maximum of 80 chars |

# Authentication Message #

The control message is defined to be

> ### _**`[:<Text-Header>:]`**_ ###

Where the _**`[:`**_ is the beginning marker and the _**`:]`**_ is the ending marker.

The field _**`<Text-Header>`**_ is defined to be

> ### _**`<version>:<msg-id>:<msg-type>:<userid>:<authentication>`**_ ###

Where the  _**:**_ is the sub-field delimiter. Each sub-field is defined in the tables below.

The content of the userid and authentication fields is related to the type of message as seen in the following table.
| **Message Type** | **Userid Field** | **Authentication Field** |
|:-----------------|:-----------------|:-------------------------|
| 77 | The user's Google email address|The OAUTH access token from Google |
| 88 | The user's ae userid|The password of the user |


# Examples #

### Initial channel validation ###
This is special request/response message pair only happens at the time the connection is just established for validating that the connection between ae-daemon and monitor is a legitimate connection.
| SOCKETMON sends | _**`[:10:143456324323234-234:22:SM:9999:00:A0:]`**_ |
|:----------------|:----------------------------------------------------|
| AE\_DAEMON replies | _**`[:10:143456324323234-234:11:AE:]`**_ |


### Hello message, it is sent by the monitor every 30 seconds ###
| SOCKETMON sends | _**`[:10:143456324345234-456:00:SM:]`**_ |
|:----------------|:-----------------------------------------|
| AE\_DAEMON replies | _**`[:10:143456324345234-456:11:AE:]`**_ |

### Report the required listening TCP port 22 is down with possible action of ignore ###
| SOCKETMON sends | _**`[:10:985765636438765-734:22:SM:0002:11:A0:22:]`**_ |
|:----------------|:-------------------------------------------------------|
| AE\_DAEMON replies | _**`[:10:985765636438765-734:11:AE:]`**_ |

### Report the blacklisted port 80 is up with possible action of log the message ###
| SOCKETMON sends | _**`[:10:465748392014328-897:22:SM:0001:11:A1:80:]`**_ |
|:----------------|:-------------------------------------------------------|
| AE\_DAEMON replies | _**`[:10:465748392014328-897:11:AE:]`**_ |

### Report the blacklisted port 80 is down (cleared) with possible action ignore ###
| SOCKETMON sends | _**`[:10:456281940583498-777:22:SM:0001:00:A0:80:]`**_ |
|:----------------|:-------------------------------------------------------|
| AE\_DAEMON replies | _**`[:10:456281940583498-777:11:AE:]`**_ |

```
```

# Protocol Message Flow between `AE_DAEMON` and `AeProxy` #
### Event update ###
| `AeProxy` sends | _**`[:10:143456324345288-812:00:AM:]`**_ |
|:----------------|:-----------------------------------------|
| AE\_DAEMON replies | Sends a list of events collected from monitors, Example: _**`[:10:465748392014328-897:22:SM:0001:11:A1:80:]`**_ _**`[:10:456281940583498-777:22:SM:0001:00:A0:80:]`**_ |

### Action event request from `aeProxy` which is initiated by `AeManager` ###
| `AeProxy` sends | _**`[:10:465748392014328-897:33:SM:0001:11:A1:80:]`**_ |
|:----------------|:-------------------------------------------------------|
| AE\_DAEMON | Acts on the action event request to _log the message_ |