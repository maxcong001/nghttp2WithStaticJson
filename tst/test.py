import asyncio
import aioh2


async def example():

    # Open client connection
    client = await aioh2.open_connection('127.0.0.1', 1234,
                                         functional_timeout=0.1)

    # Optionally wait for an ack of tickless ping - a.k.a. until functional
    await asyncio.sleep(0.1)  # simulate a busy client with something else
    rtt = await client.wait_functional()
    if rtt:
        print('Round-trip time: %.1fms' % (rtt * 1000))

    # Start request with headers
    stream_id = await client.start_request([
        (':scheme', 'h2c'),
        (':authority', 'example.com'),
        (':method', 'GET'),
        (':path', '/index.html'),
    ])

    # Send my name "world" as whole request body
    await client.send_data(stream_id, b'hello', end_stream=False)
    await client.send_data(stream_id, b'world', end_stream=True)

    # Receive response headers
    headers = await client.recv_response(stream_id)
    print('Response headers:', headers)

    # Read all response body
    resp = await client.read_stream(stream_id, -1)
    print('Response body:', resp)

    # Read response trailers
    trailers = await client.recv_trailers(stream_id)
    print('Response trailers:', trailers)

    client.close_connection()
    await asyncio.sleep(.1)


asyncio.get_event_loop().run_until_complete(example())
