#!/usr/bin/env python3

import os
import sys
import http

from http.server import *

class GZHTTPRequestHandler(SimpleHTTPRequestHandler):

    def send_head(self):
        self.original_path = self.path
        path = self.translated_path = super().translate_path(self.path)
        self.content_type = super().guess_type(self.translated_path)
        self.content_encoding = None
        accepted = []
        if 'Accept-Encoding' in self.headers:
            accepted = self.headers['Accept-Encoding']
            accepted = [ e.strip() for e in accepted.split(',') ]
        pathgz = path + '.gz'
        if os.path.isfile(pathgz) and 'gzip' in accepted:
            self.translated_path = pathgz
            self.content_encoding = 'gzip'
        return super().send_head()

    def translate_path(self, path):
        if path == self.original_path:
            return self.translated_path
        return super().translate_path(path)
    
    def guess_type(self, path):
        if path == self.translated_path:
            return self.content_type
        return super().guess_type(path)

    def send_response_only(self, code, message=None):
        super().send_response_only(code, message)
        if code == http.HTTPStatus.OK:
            # disable caching
            self.send_header('Cache-Control', 'no-store, must-revalidate')
            self.send_header('Expires', '0')
            # set content encoding
            if self.content_encoding:
                self.send_header('Content-Encoding', self.content_encoding)
            
    
if __name__ == '__main__':
    import argparse
    import contextlib

    directory = os.path.dirname(__file__ or os.getcwd())
    directory = os.path.normpath(os.path.join(directory, 'html'))

    parser = argparse.ArgumentParser()
    parser.add_argument('-b', '--bind', metavar='ADDRESS', default='127.0.0.1',
                        help='bind to this address (default: all interfaces)')
    parser.add_argument('-d', '--directory', default=directory,
                        help='serve this directory (default: %(default)s)')
    parser.add_argument('-p', '--protocol', metavar='VERSION',
                        default='HTTP/1.0',
                        help='conform to this HTTP version (default: %(default)s)')
    parser.add_argument('port', default=8000, type=int, nargs='?',
                        help='bind to this port (default: %(default)s)')
    args = parser.parse_args()
    handler_class = GZHTTPRequestHandler

    # copied from http.server
    class DualStackServer(ThreadingHTTPServer):
        def server_bind(self):
            # suppress exception when protocol is IPv4
            with contextlib.suppress(Exception):
                self.socket.setsockopt(
                    socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, 0)
            return super().server_bind()
        def finish_request(self, request, client_address):
            self.RequestHandlerClass(request, client_address, self,
                                     directory=args.directory)

    http.server.test(
        HandlerClass=handler_class,
        ServerClass=DualStackServer,
        port=args.port,
        bind=args.bind,
        protocol=args.protocol,
    )
