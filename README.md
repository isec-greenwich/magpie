# MAGPIE - Monitoring AGainst cyber PhysIcal thrEats

This is the repository for the MAGPIE prototype draft code and attack datasets developed. There exists a number of outstanding development and minor bug issues requiring updates, but these should be easily rectified if required by researchers and developers. Some of these issues are highlighted below, but more may be added in the near future. However, the attack datasets are complete and can be utilised without issue. 

Current development issues/improvements required:

- Code refactoring and advanced comments
- Redundant prototype code to be pruned and commented-out code to be removed
- User guide to be generated in separate README for offline testing, online testing
- User guide for datasets usage and manipulation to be generated
- May be more efficient to use Pyshark wrapper than separate tshark processes, in order to spawn multiple python subprocesses for each collection interface requiring the tshark display filter stdin
- Redundant push_feed.py script for online MAGPIE prototype scripts to be removed and ZMQ functionality integrated into parser C code

Important bugs/issues to address:

- Currently there is exists artifacts in the code for the proof-of-concept research this project is related to. These artifacts need pruning and may reduce the readiblity of the code in places.

Deployment specific code bugs:

- Currently a rare bug in parser code sometimes occurs and causes first data stream window to skip in online MAGPIE prototype scripts, this is likely a timing or buffering problem aligning the parsing to the ZMQ pub-sub synchronisation
- There is a bug with ZigBee Jackdaw hardware which generates increasing CRC errors and then fails after ~5-10 mins of operation, this is independent of MAGPIE code and resides in the RZRAVEN USB stick hardware used in testing.
- The shell script for automated ZigBee Jackdaw interface firmware configuration is unreliable and sometimes fails (resort to manual configuration for testing), OpenSniffer hardware likely a better interface than Jackdaw for this purpose going forward, or other ZigBee sniffing hardware with automated configuration software capabilities.


Limitations (related to Raspberry PI3 CPU):

- Excessive TCP connections from web browser sessions (e.g., dynamic content loaded, tracking ads, etc.) can cause CPU saturation during parsing, therefore filtering out specific IP addresses or specifically host to external host communication may be required. Browser sessions are less useful to monitor in the case of tracking the smart home attack sessions in this case.
- Currently tshark processes need to be isolated from stateful dumpcap process which eventually saturates memory, this is a simple task needed to define a dumpcap process within a time/size bounded pcap which is synchronised with the tshark processes reading the file into the MAGPIE C parser. This problem is an inherent issue with stateful pcap collection in tshark storing in memory. Separating dumpcap and tshark parsing processes will resolve this problem.
