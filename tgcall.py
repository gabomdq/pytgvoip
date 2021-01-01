#!/usr/bin/env python3
# Telegram VOIP calls from python example
# Author Gabriel Jacobo https://mdqinc.com

import logging
import argparse
import os
import json
import base64
from telegram.client import Telegram
from tgvoip import call_start


def setup_voip(data):
    # state['config'] is passed as a string, convert to object
    data['state']['config'] = json.loads(data['state']['config'])
    # encryption key is base64 encoded
    data['state']['encryption_key'] = base64.decodebytes(data['state']['encryption_key'].encode('utf-8'))
    # peer_tag is base64 encoded
    for conn in data['state']['connections']:
        conn['peer_tag'] = base64.decodebytes(conn['peer_tag'].encode('utf-8'))
    call_start(data)

def handler(msg):
    #print ("UPDATE >>>", msg)
    if msg['@type'] == 'updateCall':
        data = msg['call']
        if data['id'] == outgoing['id'] and data['state']['@type'] == 'callStateReady':
            setup_voip(data)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('api_id', help='API id')  # https://my.telegram.org/apps
    parser.add_argument('api_hash', help='API hash')
    parser.add_argument('phone', help='Phone nr originating call')
    parser.add_argument('user_id', help='User ID to call')
    parser.add_argument('dbkey', help='Database encryption key')
    args = parser.parse_args()

    tg = Telegram(api_id=args.api_id,
                api_hash=args.api_hash,
                phone=args.phone,
                td_verbosity=5,
                files_directory = os.path.expanduser("~/.telegram/" + args.phone),
                database_encryption_key=args.dbkey)
    tg.login()

    # if this is the first run, library needs to preload all chats
    # otherwise the message will not be sent
    r = tg.get_chats()
    r.wait()


    r = tg.call_method('createCall', {'user_id': args.user_id, 'protocol': {'udp_p2p': True, 'udp_reflector': True, 'min_layer': 65, 'max_layer': 65} })
    r.wait()
    outgoing = r.update

    tg.add_handler(handler)
    tg.idle()  # blocking waiting for CTRL+C


