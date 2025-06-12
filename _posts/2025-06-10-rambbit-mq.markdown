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

````
# stops the locally running RabbitMQ node
brew services stop rabbitmq
```

or CLI tools directly:

```
/opt/homebrew/sbin/rabbitmqctl shutdown
```



## Python Implementation 

producer.py

```
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

```
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

print("start consuming messages")
channel.start_consuming()

# Close the connection
# connection.close()  # Not needed here as we are consuming messages indefinitely

```

## Web Dashboard 

Go to http://localhost:15672/

Enter username and password, both `guest`

![](/img/post/rmq/1.png)