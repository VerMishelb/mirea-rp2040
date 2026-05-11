#!/usr/bin/env python3
import argparse
import socket
import sys
import time

try:
    from opcua import Client
except ImportError:
    print("Python package 'opcua' is not installed.")
    print("Install it with: pip install opcua")
    sys.exit(1)


def parse_endpoint(endpoint: str):
    if not endpoint.startswith("opc.tcp://"):
        raise ValueError("endpoint must start with opc.tcp://")
    tail = endpoint[len("opc.tcp://"):]
    host_port = tail.split("/", 1)[0]
    if ":" in host_port:
        host, port = host_port.rsplit(":", 1)
        return host, int(port)
    return host_port, 4840


def check_tcp(host: str, port: int, timeout: float):
    print(f"[1] TCP check {host}:{port}")
    with socket.create_connection((host, port), timeout=timeout):
        print("    TCP OK")


def main():
    parser = argparse.ArgumentParser(description="Test RP2040 open62541 OPC UA server")
    parser.add_argument("--endpoint", default="opc.tcp://192.168.1.2:4840")
    parser.add_argument("--node", default="ns=1;s=TestValue")
    parser.add_argument("--timeout", type=float, default=5.0)
    parser.add_argument("--write", type=int, default=None, help="Optional Int32 value to write to TestValue")
    args = parser.parse_args()

    host, port = parse_endpoint(args.endpoint)
    try:
        check_tcp(host, port, args.timeout)
    except Exception as exc:
        print(f"TCP failed: {exc}")
        return 2

    print(f"[2] OPC UA connect {args.endpoint}")
    client = Client(args.endpoint, timeout=args.timeout)
    try:
        started = time.time()
        client.connect()
        print(f"    OPC UA connected in {time.time() - started:.2f}s")

        print("[3] NamespaceArray")
        try:
            print("   ", client.get_node("ns=0;i=2255").get_value())
        except Exception as exc:
            print(f"    NamespaceArray read failed: {exc}")

        node = client.get_node(args.node)
        if args.write is not None:
            print(f"[4] Write {args.node} = {args.write}")
            node.set_value(args.write)

        print(f"[5] Read {args.node}")
        value = node.get_value()
        print("\n========== RESULT ==========")
        print(f"Endpoint : {args.endpoint}")
        print(f"NodeId   : {args.node}")
        print(f"Value    : {value!r}")
        print("============================\n")
        return 0
    except Exception as exc:
        print("\n========== ERROR ==========")
        print(f"{type(exc).__name__}: {exc}")
        print("===========================\n")
        return 3
    finally:
        try:
            client.disconnect()
        except Exception:
            pass


if __name__ == "__main__":
    sys.exit(main())
