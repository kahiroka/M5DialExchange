from argparse import ArgumentParser
import paho.mqtt.client as mqtt
 
def on_connect(client, userdata, flag, rc):
    client.subscribe("kabusapi/" + userdata)

def on_disconnect(client, userdata, rc):
    pass

def on_message(client, userdata, msg):
    print(msg.payload.decode('utf-8'))

def getArgs():
    usage = 'python3 {}'.format(__file__)
    argparser = ArgumentParser(usage=usage)
    argparser.add_argument('-i', '--ip', nargs='?', default='localhost', type=str, dest='host', help='mqtt broker ip address')
    argparser.add_argument('-p', '--port', nargs='?', default=1883, type=int, dest='port', help='mqtt broker port#')
    argparser.add_argument('-t', '--timeout', nargs='?', default=60, type=int, dest='timeout', help='mqtt connection timeout')
    argparser.add_argument('--topic', nargs='?', default='#', type=str, dest='topic', help='mqtt topic (board, exchange or #(default))')
    return argparser.parse_args()

def main(): 
    args = getArgs()
    client = mqtt.Client(userdata=args.topic)
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message
    client.connect(args.host, args.port, args.timeout)
    client.loop_forever()

if __name__ == '__main__':
    main()
