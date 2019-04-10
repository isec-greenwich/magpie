import argparse
import magpie_run as run
from colorama import init
import sys
init(strip=not sys.stdout.isatty())  # strip colors if stdout is redirected
# from termcolor import cprint
# from pyfiglet import figlet_format
# import get_rpp
from datetime import timedelta

if __name__ == '__main__':
    # Script mode parameters
    parser = argparse.ArgumentParser(description='MAGPIE runtime parameters')
    parser.add_argument('-r', '--riskprofile', type=int,
                        help='household risk profile policy', required=False)
    parser.add_argument('-w', '--window', type=int,
                        help='Monitoring time window interval', required=False)
    parser.add_argument('-a', '--activity', type=int,
                        help='parsing windowing interval', required=False)
    parser.add_argument('-c', '--collect', type=int,
                        help='collection time bin for MDs sample colletion', required=False)
    parser.add_argument('-f', '--filename', type=str,
                        help='training data and model file prefixes', required=False)
    parser.add_argument('-m', '--mode', type=int,
                        help='runtime mode', required=False)
    parser.add_argument('-p', '--activity_presence', type=int,
                        help='activity presence state', required=False)

    args = parser.parse_args()

    # MAGPIE interfaces
    feeds = ['ip', 'wifi', 'zigbee', 'audio', 'rf', 'amds']

    window = args.window
    activity_check = args.activity  # 1 = ACC, 2 = SPC, 3 = RECALL
    rpp = args.riskprofile  # 1 = ACC, 2 = SPC, 3 = RECALL
    collect_bin = args.collect
    mdt = timedelta(seconds=window)  # currently not used
    filename = args.filename
    mode = args.mode
    activity_presence = args.activity_presence

    if rpp == 1:
        rpp_str = 'Monitoring Sensitivity: Balanced'
    elif rpp == 2:
        rpp_str = 'Monitoring Sensitivity: Low'
    else:
        rpp_str = 'Monitoring Sensitivity: High'

    # create instance of main MAGPIE monitoring class
    run = run.Monitor()

    # initiate the real-time monitoring method
    run.run(rpp_str, mdt, feeds, activity_check, collect_bin, filename, mode, activity_presence)

