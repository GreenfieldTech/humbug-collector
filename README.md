Humbug Collector for Asterisk Open Source PBX
=============================================
The Humbug Collector agent is used to connect your existing Asterisk PBX system to the Humbug Call Analytics and Fraud Analysis cloud.
The collector can be used as a means to connect to the Humbug Service, or, as a means to connect your Asterisk system to an external auditing facility.

Changelog:
----------
0.8.2 - Initial Humbug Collector Release, by Humbug Telecom Labs Ltd
0.9.0 - Controllable Instance Release, by Greenfield Technlogies Ltd

Installation
------------
Download from Github:

```
git clone https://github.com/GreenfieldTech/humbug-collector.git
```

Now, you are required to compile and installation the code
```
cd humbug-collector
make all
make install
make config
```

Asterisk Configuration
----------------------
In order to activate the agent, you are required to add a manager entry to your Asterisk manager.conf - similar to the below:

```
[humbug]
secret = Your_Super_Secret_Password
permit = 127.0.0.1/255.255.255.0
read = all 
write = 
```

Now, you are required to enable CDR logging to your manager interface. Edit the /etc/asterisk/cdr.conf file to match the following items:
```
enable=yes
.
.
unanswered = yes
```

Then, you need to enable cdr_manager in the cdr_manager.conf file:
```
[general]
enabled = yes
```

Now, logon to your Asterisk console and issue the following commands:
```
asterisk*CLI> manager reload 
  == Parsing '/etc/asterisk/manager.conf':   == Found
  == Parsing '/etc/asterisk/manager_additional.conf':   == Found
  == Parsing '/etc/asterisk/manager_custom.conf':   == Found
asterisk*CLI> module reload cdr_manager.so 
    -- Reloading module 'cdr_manager.so' (Asterisk Manager Interface CDR Backend)  
```

Following the above procedure, issue the following command to verify your CDR backend is loaded correctly:

```
asterisk*CLI> cdr show status

Call Detail Record (CDR) settings
----------------------------------
  Logging:                    Enabled
  Mode:                       Simple
  Log unanswered calls:       Yes

* Registered Backends
  -------------------
    cdr-custom
    mysql
    csv
    cdr_manager

```

Your output should resemble the above - if it doesn't, you've done something wrong.

Collector Configuration
-----------------------
The collector has a single configuration file, located at /etc/humbug/humbug.conf. Following below are the parameters you are required to modify, in order to get up and running:

```
# Astersk manager connection

# IP address
address = 127.0.0.1

# Manager port
port=5038

# Manager username
user = humbug

# Manager secret
secret = Your_Super_Secret_Password
```
Make sure you setup your Humbug PBX key correctly. Just to make sure, your API key appears in the Humbug console, under Config->Settings->My PBXs. Pay carefull attention here, there are two keys configured for each PBX. The **h_apikey** parameter requires the first key (**the one on the left**).

```
h_apikey = The_Key_Appearing_In_The_First_Box
```
Now, if you wish to encrypt your data, you should set the following:
```
# Send encrypted data
# no by default
encrypted = yes

# Encryption key
h_key = Your_Key_Appearing_In_The_Second_Box
```
Please note, the following are currently not in use:
```
# Check community blacklist
community_blacklist = no

# Do Hangup
action_hangup = no 
```
Once you have everything up and running smoothly, you can lower your logging level:
```
# Log file
log_file = /var/log/humbug/humbug-collector.log

# Debug level:
#       1 - ERROR               Show errors only
#       2 - WARNING             Show errors and warnings
#       3 - INFO                Show errors, warnings and info
#       4 - DEBUG               Show all ( Huge data )
debug_level = 1
```
Multiple Instances
------------------
It is now possible to run multiple instances of the Humbug Collector on the same machine. Pay attention that in order to do so, you are required to create multiple configuration files, change the **log_file** settings in the configuration file, then create a proper init script for each instance. 

License
-------
GPL v2


**Free Software, Hell Yeah!**

[Humbug Telecom Labs]:http://www.humbuglabs.org/
[Greenfield Technologies]:http://www.greenfieldtech.net/
[Nir Simionovich]:http://www.simionovich.com
