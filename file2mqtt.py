from argparse import ArgumentParser
import sys
import paho.mqtt.client as mqtt
import time

def on_connect(client, userdata, flag, rc):
    print('on connect : ' + str(rc))

def on_disconnect(client, userdata, rc):
    print('on disconnect : ' + str(rc))

def on_publish(client, userdata, mid):
    pass

def getArgs():
    usage = 'python3 {}'.format(__file__)
    argparser = ArgumentParser(usage=usage)
    argparser.add_argument('-i', '--ip', nargs='?', default='localhost', type=str, dest='host', help='mqtt broker ip address')
    argparser.add_argument('-p', '--port', nargs='?', default=1883, type=int, dest='port', help='mqtt broker port#')
    argparser.add_argument('-t', '--timeout', nargs='?', default=60, type=int, dest='timeout', help='mqtt connection timeout')
    argparser.add_argument('-f', '--file', nargs='?', type=str, dest='file', help='mqtt log file')
    argparser.add_argument('--topic', nargs='?', default='board', type=str, dest='topic', help='mqtt topic (board or exchange)')
    argparser.add_argument('--loop', action='store_true', dest='loop', help='infinitely loop the file')
    return argparser.parse_args()

def main():
    args = getArgs()
    if not args.file:
        print('file is not specified')
        sys.exit()

    global client
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_publish = on_publish
    client.connect(args.host, args.port, args.timeout)
    client.loop_start()

    while True:
        with open(args.file, 'r', encoding='utf-8') as f:
            print('begin')
            for line in f:
                client.publish('kabusapi/' + args.topic, line.strip())
                time.sleep(0.1)
            print('end')

        if not args.loop:
            break

if __name__ == '__main__':
    main()
