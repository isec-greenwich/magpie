import zmq
import json
import sys
import argparse


def producer():
	list1 = []
	list2 = []
	context = zmq.Context()
	zmq_socket = context.socket(zmq.PUSH)
	zmq_socket.bind("tcp://127.0.0.1:%s" % port)

	def send(list):
		print('[DEBUG] sending: ' + str(feed))
		zmq_socket.send_string(json.dumps(list))

	while True:
		line = sys.stdin.readline()
		row = line.split(",")
		row[-1] = row[-1].strip("\n")
		if row[-1] == "0":
			if list2:
				send(list2)
				list2[:] = []
			else:
				pass
			row[0] = row[0].strip("()")
			list1.append(row)
		elif row[-1] == "1":
			if list1:
				send(list1)
				list1[:] = []
			else:
				pass
			row[0] = row[0].strip("()")
			list2.append(row)


if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='attack test capture')
	parser.add_argument('-p', '--port', type=str, help='push port', required=True)
	parser.add_argument('-f', '--feed', type=str, help='push port', required=True)
	args = parser.parse_args()
	port = args.port
	feed = args.feed

	producer()
