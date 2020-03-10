# MAGPIE: The Intrusion Detection System for Cyber-Physical Attacks in Internet of Things Smart Homes

MAGPIE (Monitoring AGainst cyber PhysIcal thrEats) is a self-configurable intrusion detection system for detecting cyber-physical attacks in smart home and IoT environments. MAGPIE employs a non-stationary UCB Reinforcement Learning algorithm and the Isolation Forest machine learning algorithm from the SCIKIT learn library, to automatically discover unknown attacks. MAGPIE enables users' to define their own threshold to optimise attack detection for lowering False Positive or False Negative detections, based on the users' detection tolerance and requirements. MAGPIE takes in consideration by cyber and physical sources of data, as well as performing presence inference to dynamically select an appropriate presence-optimised attack detection model at runtime.

# Included in this repository:
1. Full Python source-code with dpkg requirements on Raspberry PI
2. The MAGPIE Parser (written in C)
3. Full list of Python and external library requirements
4. User & Developer user-guide - <strong>To be completed</strong>
5. All normal and attack datasets used in smart home experiments for reproducing experiments

# MAGPIE core Features:
1. Online and offline training of MAGPIE system
2. Offline detection testing and verification of MAGPIE system
3. Online monitoring of cyber+physical datastream collection interfaces
4. Configurable Reinforcement Learning settings for Isolation Forest model training
5. Configurable online monitoring time window (e.g., datastream buffering and aggregation)
6. Modular datastream parsing engine to add/remove feeds (requires regex extraction for input)
7. User-defined attack detection threshold parameters
8. Configurable user presence detection for dynamic presence model selection during online monitoring
9. Interactive monitoring console output

# Intrusion Detection System Architecture:
The intrusion detection system architecture of MAGPIE is presented below, detailing each of the software and functional components.
<br><br><img src="https://github.com/isec-greenwich/magpie/blob/master/v1.0/magpie_architecture.png" width="80%" height="80%"><br><br>

# Raspberry PI3 Prototype Schematic:
The schematic below provides an overview of the RaspberryPI 3 prototype system and hardware extensions to implement the MAGPIE architecture and intrusion detection software platform.
<br><br><img src="https://github.com/isec-greenwich/magpie/blob/master/v1.0/magpie_proto.png" width="50%" height="50%"><br><br>

# Smart Home Testbed:
Below is a the configuration overview of the smart home testbed used to evaluate the MAGPIE prototype system, including smart home IoT devices which were used in experiments and present in the datasets.

<br><br><img src="https://github.com/isec-greenwich/magpie/blob/master/v1.0/magpie_testbed.png" width="75%" height="75%"><br><br>

# License [MIT]

[MIT]: https://github.com/isec-greenwich/magpie/blob/add-license-1/LICENSE.md
