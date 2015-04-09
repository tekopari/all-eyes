# `ae` Password File #
The `/etc/ae/password` file stores the user credentials for an ae user.  Credentials include the username hash, salt, and password hash.  This file is used to authenticate the user's of `aeManager` when they send message to the aeProxy.

# `ae` Authorization File #
The `/etc/ae/auth` file is an access control list that contains the list of authorized users of `ae`.  Users must be authenticated and authorized to use the `ae` system before access is granted.

# Methods of Authorization #
There are two methods of authentication.
  1. The first method is a simple username and password mechanism for developers of the ae system that lack android devices.  The `aeManager` if it detects that Google Play Services are not installed it will `automatically` switch over to use this method.  To use this method the user must have an ae user account and be authorized to use ae (see below).
  1. The second method uses the `GoogleAuthUtil` api offered from Google Play Services.  This method obtains an `OAUTH` access token from Google that is passed to the `aeProxy`.  The `aeProxy` uses this token to authenticate the user with Google.

# Creating an `ae` username and password account #
  1. Log onto the `AeProxy` host
  1. Add the user to the system with the command: _`"sudo java -cp /usr/local/bin/AeProxy.jar org.tbrt.ae.AeAuthentication -add USERNAME"`_ where USERNAME is the name of the user account to create.
  1. At the password prompt enter the user's password
  1. Add this username and password to the setting in the aeManager

# Granting `ae` access to an `ae` user #
  1. Log onto the `AeProxy` host
  1. Add the user to the system with the command: _`"sudo java -cp /usr/local/bin/AeProxy.jar org.tbrt.ae.AeAuthentication -auth USERNAME"`_ where USERNAME is the name of the user account created above.

# Granting `ae` access to a Google User #
  1. Log onto the `AeProxy` host
  1. Add the user to the system with the command: _`"sudo java -cp /usr/local/bin/AeProxy.jar org.tbrt.ae.AeAuthentication -auth GOOGLE_EMAIL_ADDRESS"`_ where `GOOGLE_EMAIL_ADDRESS` is the user's gmail address.