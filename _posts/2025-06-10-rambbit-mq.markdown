---
layout:     post
title:      "Rabbit MQ"
date:       2025-06-10 20:00:00
author:     "Becks"
header-img: "img/post-bg-rwd.jpg"
catalog:    true
tags:
    - System Design
    - Grook
---

## AMQP

RabbitMQ is a implementation of **AMQP (Advanced Message Queueing Protocol)** message model. 

AMQP uses a remote procedure call to allow one computer to execute programs or methods on another computer(比如broker). Two way communication (both broker and client can use rpc to run programs or call each others). 和Object-oriented programing一样，rabbitmq use commands which consists of classes and methods to communicate between clients and brokers

AMQP (Advanced Message Queuing Protocol) is a standardized protocol used for asynchronous, secure, and reliable message exchange between applications. It enables systems to communicate and exchange messages in a way that's robust, interoperable, and scalable. 

Key aspects of AMQP:

- **Interoperability**: AMQP allows different applications, even those written in different programming languages, to communicate effectively. 
- **Asynchronous Communication**: AMQP facilitates asynchronous messaging, where a sender can send a message without waiting for immediate acknowledgement from the receiver. This is achieved through message brokers and queues. 
- **Reliability**: AMQP provides mechanisms for ensuring that messages are delivered reliably, even if there are network issues or errors. 
- **Security**: AMQP supports secure communication through protocols like TLS. 
- **Routing and Queuing**: AMQP uses message brokers, exchanges, and queues to <span style="color:purple">route messages to the correct recipients</span>. 
- **Open Standard**: AMQP is an open standard defined by OASIS and ISO - International Organization for Standardization. 


**Components of AMQP:**

- **Message Brokers**: AMQP relies on <span style="background-color:#FFFF00">**brokers to act as intermediaries between senders and receivers**</span>, handling message routing and delivery. 
- Producer and Consumer
  - Producer sends messages to the broker.
  - Consumer receives messages from the broker.
- **Exchanges**: Exchanges are responsible for <span style="background-color:#FFFF00">**routing messages to queues**</span> <span style="color:purple">**based on predefined rules and bindings**</span>. A message routing agent in the broker.
  - recieve messages and distribute how they're addressed
  - 可以连接many queues
- **Queues**: Queues act as storage for messages, allowing consumers to retrieve them at their own pace. 
- **Bindings**: <span style="background-color:#FFFF00">**Bindings define the relationship between exchanges and queues**</span>, <span style="background-color:#FFFF00">**specifying how messages are routed. often with a routing key to filter messages**</span>
- **Routing key**: <span style="background-color:#FFFF00">**A string used to determine how messages are routed to queues**</span>.
- **Messages**: Messages are the <span style="color:purple">**data units**</span> exchanged between applications, containing both content and metadata. 
- **Delivery Guarantees**. AMQP supports different delivery modes:
  - **At-most-once**
  - **At-least-once**
  - **Exactly-once** (though this is complex and often simulated)

#### AMQP Frame type 

- Method Frame: has class and method
- Content Header Frame
- Body Frame
- Heartbeat Frame

![](/img/post/rmq/2.png)

AMQP define exactly what should be contained each bytes of frame

40 表示exchange class, method 10 -> 表示declare method in exchange class

when sending or receiving a message through rabbitmq, the first frame always is a method frame which corresponds to sending/recieving method


![](/img/post/rmq/3.png)

The doc for the AMQP specification gives a full which id for which methods and short description which methods do

| Frame	  | Frame	  |    Description  | 
| :-------------:| :-------------:|:------|
| **Method**	| 1	| Carries AMQP commands like queue.declare, basic.publish, basic.consume, etc. |
| **Header** |	2 |	Carries metadata about the message body, such as content type, delivery mode, and body size. |
| **Body** |	3	| Carries the actual message data (can be split across multiple body frames).| 
| **Heartbeat**	| 8	| Used to keep the connection alive and detect dead peers.|
| **Heartbeat (reserved)**	| —	| Sometimes shown separately, but technically type 8. |
| **Frame End** |	—	 |Each frame ends with a constant byte 0xCE to mark the end of the frame. |


## Key Concepts

- **Producer**: Application that sends the messages. sending messages to the RabbitMQ server
- **Consumer**: Application that receives the messages.reading and processing messages from queue
- **Queue**: In RabbitMQ, a Queue can be referred to as <span style="background-color:#FFFF00">**temporary or buffered storage of messages**</span>, RabbitMQ manages a queue, where messages are stored before they are consumed by the receiver. <span style="background-color:#FFFF00">**Queues can have the nature of durable or transient**</span> which means persisting across restarts and deleted when the server restarts respectively.
- **Connection**: A TCP connection between your application and the RabbitMQ broker.
- **Channel**: Lightweight connections that share a single TCP connection. Publishing or or consuming messages from a queue is done over a channel.
- **Exchange**: 
  - An exchange is responsible for <span style="background-color:#FFFF00">**routing messages among queues**</span>. RabbitMQ provides support for various kinds of exchanges such as direct, fanout, topic, and headers, each queue with its logic.
    - **Direct Exchange**: In this exchange, messages are being routed based on routing keys. <span style="background-color:#FFFF00">**This means that when the routing key matches the binding key**</span>, the message gets delivered to its corresponding queue.
    - **Fanout Exchange**: With this exchange, RabbitMQ does the <span style="background-color:#FFFF00">**broadcasts to all messages**</span> that are bound to it regardless of the routing key.
    - **Topic Exchange**: With topic exchange message routing happens based on pattern matching to the routing and binding key. Routes to queues based on <span style="background-color:#FFFF00">**pattern matching**</span> (*.error, app.#). Routing key and binding keys可以是partial matches
    - **Headers Exchange**: In this <span style="background-color:#FFFF00">**route messages are based on headers instead of the routing key**</span>. <span style="background-color:#FFFF00">**The headers exchange is more flexible but less efficient**</span> compared to other exchange types.
    - **Default exchange**: not AMQP, 只是rabbitmq support, The routing is tied with name of exchange. 比如 routing key 是 “inv”, 有个queue name是 "inv", route to there 
  - Receives messages from producers and pushes them to queues depending on rules defined by the exchange type. <span style="background-color:#FFFF00">**A queue must be bound to at least one exchange**</span> to receive messages.
- Binding: Bindings are rules that exchanges use (among other things) to route messages to queues.
- Routing key: In RabbitMQ a<span style="background-color:#FFFF00">** connection between queue and exchange is called binding**</span>. The message route from the exchange to the queue is defined by binding based on specific criteria such as routing keys. A key that the exchange uses to decide how to route the message to queues. Think of the routing key as an address for the message.
- Users: It is possible to connect to RabbitMQ with a given username and password. Users can be assigned permissions such as rights to read, write, and configure privileges within the instance. Users can also be assigned permissions for specific virtual hosts.
- Vhost, virtual host: Virtual hosts provide logical grouping and separation of resources. Users can have different permissions to different vhost(s), and queues and exchanges can be created so they only exist in one vhost.



#### How RabbitMQ Works

In the first step, the producer sends a message to RabbitMQ and the exchange receives the message. <span style="background-color:#FFFF00">**Then the exchange routes the message to one or more queues based on the routing key and exchange typ3**</span>. Once the message is in the queue, it waits for the consumer to receive and process it. Once the process starts, the <span style="background-color:#FFFF00">**consumer sends an acknowledgment**</span> to RabbitMQ which indicates that the message has been handled successfully. <span style="background-color:#FFFF00">**In case of no acknowledgment, RabbitMQ may resend the message**</span> to ensure the successful full process of the message.

RabbitMQ provides support for multiple messaging patterns such as Point-to-Point, Publish/Subscribe, and Request/Reply.

- **Point-to-Point**: In this pattern, one producer sends the message to only one queue, which will be consumed by one consumer only.
- **Publish/Subscribe**: In this pattern, the producer sends the message to an exchange, which routes to multiple queues. <span style="background-color:#FFFF00">**Here each queue can have one or more consumers**</span>.
- **Request/Reply**: Here, the consumer processes the message that it has received and sends a response back to the producer, this can be referred to as **two-way communication** also.

#### Advantages of using RabbitMQ

After looking at the introduction and the way RabbitMQ works, now let's take a deep dive into the advantages of using RabbitMQ.

- **Reliable**: RabbitMQ supports mechanisms such as message <span style="background-color:#FFFF00">**durability, acknowledgment, and persistence**</span> to make sure that messages are not lost even if the server crashes.
- **Scalable**: RabbitMQ is capable of **handling thousands of messages per second**, which makes it well-suited for large-scale applications(distributed systems). With the support of clustering, multiple RabbitMQ servers can work together to increase capability and capacity.
  - 是async 不是sync, Synchronous messaging is possible but impacts scalability.  If a publisher has to wait for its recipients to respond, then it will be limited in how much it can achieve at any given time.
- **Flexible**: With multiple messaging pattern support, RabbitMQ can be used for multiple use cases. It can also be integrated with multiple protocols such as AMQP, MQTT, and STOMP.
- **Easy to use**: RabbitMQ offers a user-friendly web-based interface that allows users to monitor queues and exchanges in real time. It provides client libraries for various programming languages such as Java, Python, and Node.js.
- **Strong community support**: RabbitMQ has a large and active community with rich documentation and tutorials. It is backed by commercial support from Pivot Software, which ensures that users can get help when needed.


对于其他的queue, broker administrator define in message model how message moves (all defined), but within RabbitMQ, <span style="background-color:#FFFF00">the moves through the system is largely a part of the **message metadata**. 是Application and developer that has a lot of control how message move through the system</span>

Another benefits of RabbitMQ is cloud friendly, 可以deploy an instance in docker or container software. It can also run as cluster, <span style="background-color:#FFFF00">**fault tolerant, high available, and high throughput**</span>
 
across language

it support **message acknowledgement**. When a message in a queue and goes to consumer, the message stays in the queue until the consumer lets the broker know that it has recieved the messeges. 

#### Channel

In RabbitMQ, a **channel** is a <span style="background-color:#FFFF00">**virtual connection inside a TCP connection**</span> (also called a connection). It is a lightweight way for clients to communicate with the broker <span style="color:purple">without opening multiple actual network connections</span>.

**Why Channels Exist**

Opening **a full TCP connection to RabbitMQ is expensive** (in terms of resources). So instead of creating a new connection for every producer or consumer:

- <span style="background-color:#FFFF00">**A single TCP connection is opened**</span>.
- <span style="background-color:#FFFF00">**Multiple channels are created over that connection**</span> to handle different tasks in parallel (e.g., publishing and consuming).

| Property	  |    Description  | 
| :-------------:|:------|
| **Lightweight** |	Much less overhead than creating a new TCP connection. |
| **Isolated** |	<span style="background-color:#FFFF00">**Errors on one channel don't affect others on the same connection.**</span> |
| **Concurrency** |	Multiple producers/consumers can operate simultaneously via separate channels.|
| **Scoped** |	<span style="color:purple">AMQP commands (e.g., declaring a queue, publishing) are always done over a channel</span>. |
| **Mandatory** |	<span style="background-color:#FFFF00">**Every RabbitMQ operation (publish, consume, declare queue/exchange) must be done via a channel**</span>. |


#### install rabbitmq 

https://www.rabbitmq.com/docs/install-homebrew 

```
brew update
brew install rabbitmq
```

#### Running and Managing the Node

**Starting a Node In the Foreground**

```
CONF_ENV_FILE="/opt/homebrew/etc/rabbitmq/rabbitmq-env.conf" /opt/homebrew/opt/rabbitmq/sbin/rabbitmq-server
```

After starting a node, we recommend enabling all feature flags on it:

```
# highly recommended: enable all feature flags on the running node
/opt/homebrew/sbin/rabbitmqctl enable_feature_flag all
```

**Starting a Node In the Background**

```
# starts a local RabbitMQ node
brew services start rabbitmq

# highly recommended: enable all feature flags on the running node
/opt/homebrew/sbin/rabbitmqctl enable_feature_flag all
```

**Stopping the Server**

```
# stops the locally running RabbitMQ node
brew services stop rabbitmq
```

or CLI tools directly:

```
/opt/homebrew/sbin/rabbitmqctl shutdown
```



## Python Implementation 

producer.py

```python
import pika 


connection_parameters = pika.ConnectionParameters("localhost")

connection = pika.BlockingConnection(connection_parameters)
#don't directly interact with the connection, use a channel instead


channel = connection.channel()

channel.queue_declare(queue="letterbox")
#use default exchange 

message = "Hello. this is my first message"

# exchange="" means the default exchange


channel.basic_publish(exchange="", routing_key="letterbox", body=message)

print(f"Sent message: {message}")
# Close the connection
connection.close()  
```

consumer.py

```python
import pika

def on_message_recieved(ch, method, properties, body):
    print(f"Received message: {body.decode()}")

connection_parameters = pika.ConnectionParameters("localhost")

connection = pika.BlockingConnection(connection_parameters)
#don't directly interact with the connection, use a channel instead

channel = connection.channel()

# Even though we declare in both producer and consumer, but rabbitMQ brokers knows decalre the queue once

channel.queue_declare(queue="letterbox")
#use default exchange 

channel.basic_consume(queue="letterbox", on_message_callback=on_message_recieved, auto_ack=True)
# when pull off from the queue, it automatically ack and no manually do that

print("start consuming messages")
channel.start_consuming()

# Close the connection
# connection.close()  # Not needed here as we are consuming messages indefinitely

```

## Web Dashboard 

Go to http://localhost:15672/

Enter username and password, both `guest`

![](/img/post/rmq/1.png)



## AMQP RabbitMQ

![](/img/post/rmq/4.png)


在connection 开始发送protocol header, <span style="background-color:#FFFF00">**only data not formatted**</span>. The protocol 通常是 AMQP + constant + protocol major version + protocol minor version + protocol revision. 

- Then server either accept or reject protocol header. 
  - <span style="background-color:#FFFF00">**如果是reject, write a valid protocol header to the open TCP socket, and close the socket. 如果是accept socket, it implements the protocol accordingly and responding with the connection start method frame**. </span>. 
- 当client recieve this, client select SLA scruty and respond connection start ok method frame. 
- Next server will send client a <span style="background-color:#FFFF00">**connection secure **</span>method，the SLA protocol by exchanging chanllenges and response until both peers have recieved sufficient information to <span style="color:purple">**authenticate**</span> each other. The connection secure method chanllenges the client to provide more information. 
- Once authenicated on both side, server will send a <span style="background-color:#FFFF00">**connection tune**</span> method frame to the client. The connection tune method data contains various different pieces of information around sever connections. <span style="background-color:#FFFF00">**比如maximum channel supported, the max frame size supported, and the desired heartbeat delay**</span>
  - client respond a connection tune ok method frame 包括了details around negotiated max number of channels, maximum frames and heartbeat delay. 
- Final is to open connection itself by sending the server connection open method frame. Then server responds with a connection openok method frame which signals to the client that the connection is ready for use. Can see connection negotiation and opening process. <span style="background-color:#FFFF00">The communication between the client and broker is mostly sync</span>. This is different than consuming messages from the broker.


![](/img/post/rmq/5.png)


Once open a connection to broker or the server, may perform different actions 比如creating exchanges or queues or binding queues to exchanges. This is done by various different method frames from client to server. 如果queue was declared and created successfully, the server will respond with a `q.declare` ok method frame. 如果有error when create queue, rabbitmq will close the channel that rpc request was issued on. 


![](/img/post/rmq/6.png)

send basic publish frame then send content header frame (包括details 比如size of messages). Finally send one or more body frames, which makes the actual content of message. <span style="background-color:#FFFF00">**The number of body frames required depends on both size of message and maximum size supported by the connection by RabbitMQ**</span>.


**Recieving messages**: two main approaches

![](/img/post/rmq/7.png)

- <span style="background-color:#FFFF00">**basic get method**</span>: provide direct access to the messages in a queue sync. 
  - server respond `Get-Empty` (如果没有) or `Get-Ok` method
  - followed by the message which contain a **content header frame** and a number of **body frame** depending on the size of messages.
  - client then respond with a basic_ack the receipt of the message (除非设置不需要ack)
  - <span style="color:red">**not ideal for rabbitmq recieving messages**</span>. We should consume them not getting them

![](/img/post/rmq/8.png)

- <span style="background-color:#FFFF00">**basic consume method**</span>: start to ask consumer which is a transient request for messages from a specific queue. Consumer last as long as the channel declared on or client cancel them 
  - broker respond Basic `consume-ok` basic frame. 当messages arrive for that consumer, consumer send a basic deliver to the client. Followed by a header frame and a number of body frames
  - client should ack messages recieved (除非no ack required)
  - <span style="background-color:#FFFF00">**this continue until channel close or client cancel consume**</span>


## Competing Consumers

![](/img/post/rmq/9.png)

what if consumer processing speed < producer write speed. More message fill up in the queue and cause trouble on broker as each message in the queue will take up a bit memory on sever. By default, rmq use round robin manner. 

This gives some benefits in terms of <span style="background-color:#FFFF00">**scability and reliable**</span>.

![](/img/post/rmq/10.png)

比如其中一个node goes down, no longer process messages, still have other consumers



producer.py

```python
import pika 
import time
import random
connection_parameters = pika.ConnectionParameters("localhost")

connection = pika.BlockingConnection(connection_parameters)
#don't directly interact with the connection, use a channel instead
channel = connection.channel()

channel.queue_declare(queue="letterbox")
#use default exchange 

messageID = 1

while True:
    message = f"Sending Message {messageID}"

    # exchange="" means the default exchange
    channel.basic_publish(exchange="", routing_key="letterbox", body=message)
    print(f"Sent message: {message}")

    time.sleep(random.randint(1, 4))  # Simulate processing time

    messageID += 1  # Increment message ID
```

consumer.py

```python
import pika
import time 
import random 

def on_message_recieved(ch, method, properties, body):
    processing_time = random.randint(1, 6)
    print(f"Received message: {body.decode()}, will take {processing_time} seconds to process")

    time.sleep(processing_time)  # Simulate processing time
    ch.basic_ack(delivery_tag=method.delivery_tag)  # Acknowledge the message
    print('Finish processing message')

connection_parameters = pika.ConnectionParameters("localhost")

connection = pika.BlockingConnection(connection_parameters)
#don't directly interact with the connection, use a channel instead
channel = connection.channel()

# Even though we declare in both producer and consumer, but rabbitMQ brokers knows decalre the queue once
channel.queue_declare(queue="letterbox")
#use default exchange 

channel.basic_qos(prefetch_count=1)  # Fair dispatch, process one message at a time, 

# 如果移走这个，remove fair dispatch mechanism (有的queue没有完成，还在继续assign给它 =》 use roundbin 不efficient)

channel.basic_consume(queue="letterbox", on_message_callback=on_message_recieved)

print("start consuming messages")
channel.start_consuming()

# Close the connection
# connection.close()  # Not needed here as we are consuming messages indefinitely

```

![](/img/post/rmq/11.png)

如果开一个producer 和 一个consumer, 可以看见queue increase

![](/img/post/rmq/12.png)

**purge queue**: click the name of queue (比如letterbox), click purge messages

开多个consumer, 开几个terminal tab同时run


## Pub/Sub

deliver message to multiple consumers (fan-out exchange), to <span style="background-color:#FFFF00">**multiple different queue (Memory used by rabbitmq doesn't store multiple copies of messages, just store once, and each queue store a reference to that message)**</span>

<span style="background-color:#FFFF00">**Completely decoupling our producer from our downstream service.**</span> Producer doesn't care if 0 services are consuming messages or if hundreds of services consuming messages. It will continue to publish them and the fan-out exchange will take care who is interested in

![](/img/post/rmq/13.png)

<span style="background-color:#FFFF00">**bind each queues to the exchange**</span>. The fanout exchange use these bindings to know what queues are interested in message in question. 上图是3 queue, 3 bindings => each messages will send to each queue

可以使用temporary queue, no need to specifically declare these queues upfront


**producer.py**

```python
import pika 
import time
from pika.exchange_type import ExchangeType
import random
connection_parameters = pika.ConnectionParameters("localhost")

connection = pika.BlockingConnection(connection_parameters)
#don't directly interact with the connection, use a channel instead
channel = connection.channel()

#channel.queue_declare(queue="letterbox")
# producer doesn't need to declare the queue, as each consumer has dedicated queue created by itself
#use default exchange 

#declare fanout exchange

channel.exchange_declare(exchange='pubsub', exchange_type='fanout')

message = f"I want to boadcast this message"

# exchange="" means the default exchange

channel.basic_publish(exchange="pubsub", routing_key="", body=message)
print(f"Sent message: {message}")
connection.close()  
```


**firstConsumer.py**

```python
import pika
def on_message_recieved(ch, method, properties, body):
    print(f"first consumer Received message: {body.decode()}, ")

connection_parameters = pika.ConnectionParameters("localhost")

connection = pika.BlockingConnection(connection_parameters)
#don't directly interact with the connection, use a channel instead

channel = connection.channel()

channel.exchange_declare(exchange='pubsub', exchange_type='fanout')

queue = channel.queue_declare(queue="", exclusive=True)  # Declare a unique queue for this consumer
# Queue = "", server randomly choose the name of the queue

# exclusive=True means the queue will be deleted when the consumer disconnects

# use default exchange 

channel.queue_bind(exchange='pubsub', queue=queue.method.queue)  
# Bind the queue to the exchange, otherwise it may not receive messages

channel.basic_qos(prefetch_count=1)  # Fair dispatch, process one message at a time

channel.basic_consume(queue=queue.method.queue, auto_ack=True, on_message_callback=on_message_recieved)

print("start consuming messages")
channel.start_consuming()

# Close the connection
# connection.close()  # Not needed here as we are consuming messages indefinitely
```

**secondConsumer.py**

```python
import pika

def on_message_recieved(ch, method, properties, body):
    print(f"second consumer Received message: {body.decode()}, ")

connection_parameters = pika.ConnectionParameters("localhost")

connection = pika.BlockingConnection(connection_parameters)
#don't directly interact with the connection, use a channel instead

channel = connection.channel()

channel.exchange_declare(exchange='pubsub', exchange_type='fanout')

queue = channel.queue_declare(queue="", exclusive=True)  # Declare a unique queue for this consumer

# Queue = "", server randomly choose the name of the queue

# exclusive=True means the queue will be deleted when the consumer disconnects

#use default exchange 

channel.queue_bind(exchange='pubsub', queue=queue.method.queue)  
# Bind the queue to the exchange, otherwise it may not receive messages

channel.basic_consume(queue=queue.method.queue, auto_ack=True, on_message_callback=on_message_recieved)

print("start consuming messages")
channel.start_consuming()

# Close the connection
# connection.close()  # Not needed here as we are consuming messages indefinitely

```



## Routing 


![](/img/post/rmq/14.png)

- Direct Exchange: use binding keys and routing keys to route the message
  - 比如publish a message to direct exchange, message routing key 必须和binding key 一样才可以
  - <span style="background-color:#FFFF00">**多个queue 可以bound to the direct exchange using the same binding key**</span>
  - <span style="background-color:#FFFF00">**A single queue 可以有多个bindings**</span> (比如上图中间的example, 两个binding keys) => <span style="background-color:#FFFF00">**Give great flexibility how to route the messages**</span>


![](/img/post/rmq/15.png)

- **Topic exchange**: 不能有arbitrary routing keys, it must be a list of words delimited by dots. 
  - 也需要binding keys, pass by routing keys
  - `*` 表示 exactly one word, or `#` 表示zero or more words
    - 比如 `user.europe.*` 表示`user.europe` 开始加上任何一个词, 比如 `user.europe.payment`
    - `user.#` 可以是以`user`开始的任何词, 可以是user, 可以是`user.eruope`, 也可以是`user.europe.payment`
  - **Achieve interesting flexibility**
