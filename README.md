# Linux-Rootkit
A Linux based rootkit designed with advanced kernel-level evasion techniques. This is the Senior Project repository for John Woodman, Owen Flannagan, and Brandon Stacy.

## Set Up

### Compile Malware
`cd Malware`
`g++ malware.cpp -lpthread -lssh -o malware -static-libstdc++ -DID=69 -DSLEEP=10 -std=c++17`

### Start Server
`cd Server`
`node app.js`

### Access C2 CLI
`cd C2`
`python3 C2.py`


