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
    - **Topic Exchange**: With topic exchange message routing happens based on pattern matching to the routing and binding key.
    - **Headers Exchange**: In this <span style="background-color:#FFFF00">**route messages are based on headers instead of the routing key**</span>. <span style="background-color:#FFFF00">**The headers exchange is more flexible but less efficient**</span> compared to other exchange types.
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
- **Flexible**: With multiple messaging pattern support, RabbitMQ can be used for multiple use cases. It can also be integrated with multiple protocols such as AMQP, MQTT, and STOMP.
- **Easy to use**: RabbitMQ offers a user-friendly web-based interface that allows users to monitor queues and exchanges in real time. It provides client libraries for various programming languages such as Java, Python, and Node.js.
- **Strong community support**: RabbitMQ has a large and active community with rich documentation and tutorials. It is backed by commercial support from Pivot Software, which ensures that users can get help when needed.
