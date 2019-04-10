# Magpie
Monitoring AGainst cyber PhysIcal thrEats

Current development issues:

- Major tidy up of code required for presentation
- Code comments to be populated
- Redundant code to be pruned and commented-out code to be removed
- User guide to be generated in separate README for offline testing, online testing
- User guide for datasets usage and manipulation to be generated
- Redundant push_feed.py script for online MAGPIE prototype scripts to be removed and ZMQ functionality integrated into parser C code

Important:

- Currently there is exists many instances of redundant or "hacked" code for the proof-of-concept research this project is related to. This needs tidying or optimising

Code bugs:

- Currently a bug in parser that causes first data stream window to skip in online MAGPIE prototype scripts
- Bug with ZigBee Jackdaw hardware which creates CRC errors and then fails after ~5-10 mins of operations
- Shell script for automated ZigBee Jackdaw interface configuration is unreliable and sometimes fails (resort to manual configuration for testing)


Limitations:

- Currently IP collection limited to TCP, as UDP traffic in parser is causing CPU saturation
- Excessive TCP connections from browser sessions is causing CPU saturation, therefore filter out on specific IP addresses or hosts is required. 