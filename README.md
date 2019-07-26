# MAGPIE - Monitoring AGainst cyber PhysIcal thrEats

This is the repository for the MAGPIE prototype draft code and attack datasets developed. There exists a number of outstanding development and administrative issues requiring address before this project is easily consumable for researchers and developers. Some of these issues are highlighted below, but more may be added in the near future. However, the attack datasets are complete and can be utilised without issue. 

Current development issues/improvements required:

- Major tidy up of code required for presentation
- Code comments to be populated
- Redundant code to be pruned and commented-out code to be removed
- User guide to be generated in separate README for offline testing, online testing
- User guide for datasets usage and manipulation to be generated
- May be more efficient to use Pyshark wrapper than separate tshark processes, in order to spawn multiple python subprocesses for each collection interface requiring the tshark display filter stdin
- Redundant push_feed.py script for online MAGPIE prototype scripts to be removed and ZMQ functionality integrated into parser C code

Important bugs/issues to address:

- Currently there is exists many instances of redundant or "hacked" code for the proof-of-concept research this project is related to. This needs tidying or optimising.

Deployment specific code bugs:

- Currently a bug in parser that causes first data stream window to skip in online MAGPIE prototype scripts, likely a timing or buffering problem aligning the parsing to the ZMQ pub-sub synchronisation
- Bug with ZigBee Jackdaw hardware which generates increasing CRC errors and then fails after ~5-10 mins of operation
- Shell script for automated ZigBee Jackdaw interface firmware configuration is unreliable and sometimes fails (resort to manual configuration for testing), OpenSniffer hardware likely a better interface than Jackdaw for this purpose


Limitations (on Raspberry PI3 CPU):

- Excessive TCP connections from browser sessions (e.g., dynamic content loaded, tracking ads, etc.) is causing CPU saturation during parsing, therefore filtering out specific IP addresses or specifically hosts is required. 
- Currently tshark processes need to be isolated from stateful dumpcap process which eventually saturates memory, this is a simple task needed to define a dumpcap process within a time/size bounded pcap which is synchronised with the tshark processes reading the file into the MAGPIE C parser.
