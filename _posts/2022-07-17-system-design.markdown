---
layout:     post
title:      "System Design"
date:       2022-07-18 20:00:00
author:     "Becks"
header-img: "img/post-bg-rwd.jpg"
catalog:    true
tags:
    - System Design
    - Grook
---

## Caching 


Caches take advantage of the locality of reference principle: <span style="background-color:#FFFF00">recently requested data is likely to be requested again. </span> They are used in almost every computing layer: hardware, operating systems, web browsers, web applications, and more.  <span style="color:red">A cache is like short-term memory: it has a limited amount of space, but is typically faster than the original data source and contains the most recently accessed items</span>. 

Caches can exist at all levels in architecture, <span style="background-color:#FFFF00">but are often found at the level nearest to the front end (**cache 通常接近front-end**)</span>, where they are implemented to return data quickly without taxing downstream levels.

#### Application server cache

Placing a cache directly on a request layer node enables the local storage of response data. <span style="background-color:#FFFF00">Each time a request is made to the service, the node will quickly return locally cached data if it exists. If it is not in the cache, the requesting node will fetch the data from the disk **先检查是否在cache, 如果不在fetch from the disk**<span>. The cache on one request layer node could also be located both in <span style="background-color:#FFFF00">memory (which is very fast) and on the node’s local disk (faster than going to network storage) **可以是在memory or local disk**</span>.

What happens when you expand this to many nodes? If the request layer is expanded to multiple nodes, it’s still quite possible to have each node host its own cache. However, if your load balancer randomly distributes requests across the nodes, the same request will go to different nodes, **thus increasing cache misses**. Two choices for overcoming this hurdle are **global caches** and **distributed caches**.

<span style="background-color:#FFFF00">针对cache miss的两种方法: **global caches** and **distributed caches**.</span>

#### Content Delivery (or Distribution) Network (CDN)#

CDNs are a kind of cache that comes into play for sites <span style="background-color:#FFFF00">**serving large amounts of static media**</span>. In a typical CDN setup, a request will first ask the CDN for a piece of static media; the CDN will serve that content if it has it <span style="background-color:#FFFF00">**locally available**</span>. <span style="color:red">If it isn’t available</span>, the CDN will <span style="background-color:#FFFF00">**query the back-end servers for the file**</span>, cache it locally, and serve it to the requesting user. (先看CDN 有没有local cache, 如果没有再query server)

If the system we are building is not large enough to have its own CDN, we can ease a future transition by serving the static media off a separate subdomain (e.g., static.yourservice.com) using a lightweight HTTP server like Nginx, and cut-over the DNS from your servers to a CDN later.

#### Cache Invalidation

If the <span style="color:red">**data is modified in the database, it should be invalidated in the cache**</span>; if not, this can cause inconsistent application behavior. Solving this problem is known as cache invalidation


<span style="background-color:#FFFF00">**Write-through cache**</span>: Under this scheme, data is <span style="color:red">**written into the cache and the corresponding database simultaneously**</span>. The cached data allows for fast retrieval and, since the same data gets written in the permanent storage, we will have <span style="color:red">**complete data consistency between the cache and the storage**</span>. Also, this scheme ensures that nothing will get lost in case of a crash, power failure, or other system disruptions. <span style="background-color:#FFFF00">**(每次都保持cache 和 DB 的协同，但是high latency for write)**</span>

Although, write-through minimizes the risk of data loss, since every write operation must be done twice before returning success to the client, this scheme has the disadvantage of <span style="background-color:#FFFF00">**higher latency**</span> for write operations.

<span style="background-color:#FFFF00">**Write-around cache**</span>: This technique is similar to write-through cache, <span style="color:red">**but data is written directly to permanent storage, bypassing the cache**</span>. This can reduce the cache being flooded with write operations that will not subsequently be re-read, but has the disadvantage that a read request for recently <span style="**background-color:#FFFF00**">**written data will create a “cache miss” and must be read from slower back-end storage and experience higher latency**</span>. <span style="background-color:#FFFF00">**(直接写入DB， pass cass,但是higher latency for read recent data, cache misses)**</span>
 
<span style="background-color:#FFFF00">**Write-back cache**</span>: Under this scheme, data is written to cache alone, and completion is immediately confirmed to the client. <span style="color:red">**The write to the permanent storage is done after specified intervals or under certain conditions**</span>. This results in <span style="color:red">**low-latency and high-throughput for write-intensive applications;**</span> however, this speed <span style="color:red">**comes with the risk of data loss**</span> in case of a crash or other adverse event because the only copy of the written data is in the cache. <span style="background-color:#FFFF00">**(全部写入cache, 特定的时间段再写入DB, 有data 丢失的风险)**</span>

#### Cache eviction policies#

- **First In First Out (FIFO)**: The cache evicts the first block accessed first without any regard to how often or how many times it was accessed before.
- **Last In First Out (LIFO)**: The cache evicts the block accessed most recently first without any regard to how often or how many times it was accessed before.
- **Least Recently Used (LRU)**: Discards the least recently used items first.
- **Most Recently Used (MRU)**: Discards, in contrast to LRU, the most recently used items first.
- **Least Frequently Used (LFU)**: Counts how often an item is needed. Those that are used least often are discarded first.
- **Random Replacement (RR)**: Randomly selects a candidate item and discards it to make space when necessary.


## What is a CDN?

A content delivery network (CDN) refers to a geographically distributed group of servers which work together to provide fast delivery of Internet content.

A CDN allows for the <span style="color:red">**quick transfer **</span>of assets needed for loading Internet content including HTML pages, javascript files, stylesheets, images, and videos. The popularity of CDN services continues to grow, and today the majority of web traffic is served through CDNs, including traffic from major sites like Facebook, Netflix, and Amazon.

A properly configured CDN may also help protect websites against some common malicious attacks, such as Distributed Denial of Service (DDOS) attacks.

#### benefits of using a CDN

- **Improving website load times** - By distributing content <span style="background-color:#FFFF00">**closer to website visitors**</span> by using a nearby CDN server (among other optimizations), visitors experience **faster page loading times**. As visitors are more inclined to click away from a slow-loading site, a CDN can reduce bounce rates and increase the amount of time that people spend on the site. In other words, <span style="background-color:#FFFF00">a faster a website means more visitors will stay and stick around longer</span>.
- **Reducing bandwidth costs** - Bandwidth consumption costs for website hosting is a primary expense for websites. Through caching and other optimizations, CDNs are able to <span style="color:red">**reduce the amount of data an origin server must provide**</span>, thus <span style="background-color:#FFFF00">**reducing hosting costs**</span> for website owners.
- **Increasing content availability and redundancy** - Large amounts of traffic or hardware failures can interrupt normal website function. Thanks to their <span style="background-color:#FFFF00">**distributed nature, a CDN can handle more traffic and withstand hardware failure**</span> better than many origin servers.
- **Improving website security** - A CDN may improve security by providing DDoS mitigation, improvements to security certificates, and other optimizations.


#### How does a CDN work?

At its core, a CDN is a network of servers linked together with the goal of delivering content as **quickly, cheaply, reliably, and securely** as possible. In order to improve speed and connectivity, a CDN will <span style="background-color:#FFFF00">**place servers at the exchange points between different networks (Server 放在所有network 中间)**</span>.

<span style="background-color:#FFFF00">These Internet exchange points (IXPs) are the primary locations where different Internet providers connect</span> in order to provide each other access to traffic originating on their different networks. By having a connection to these high speed and highly interconnected locations, a CDN <span style="background-color:#FFFF00">provider is able to reduce costs and transit times in high speed data delivery</span>.

![](/img/post/system-design/CDN.png)

Beyond placement of servers in IXPs, a CDN **makes a number of optimizations on standard client/server data transfers**. CDNs <span style="background-color:#FFFF00">**place Data Centers at strategic locations**</span> across the globe, enhance security, and are designed to survive various types of failures and Internet congestion.


#### Latency - How does a CDN improve website load times?


- The globally distributed nature of a CDN means <span style="background-color:#FFFF00">**reduce distance between users and website resources (减少user 和website resource 距离)**</span>. Instead of having to connect to wherever a website’s origin server may live, a CDN lets users <span style="background-color:#FFFF00">**connect to a geographically closer data center**</span>. Less travel time means faster service.
- Hardware and software optimizations such as <span style="background-color:#FFFF00">**efficient load balancing and solid-state hard drives**</span> can help data reach the user faster.
- CDNs can reduce the amount of data that’s transferred by <span style="background-color:#FFFF00">**reducing file sizes using tactics such as minification and file compression**</span>. Smaller file sizes mean quicker load times.
- CDNs can also <span style="color:red">speed up sites which use TLS/SSL certificates by optimizing connection reuse and enabling TLS false start</span>.

#### Reliability and Redundancy - How does a CDN keep a website always online?

Uptime is a critical component for anyone with an Internet property. Hardware failures and spikes in traffic, as a result of either malicious attacks or just a boost in popularity, have the potential to bring down a web server and prevent users from accessing a site or service. A well-rounded CDN has several features that will minimize downtime:

- <span style="background-color:#FFFF00">**Load balancing distributes network traffic evenly across several servers （Load balancing evenly）**</span>, making it easier to scale rapid boosts in traffic.
- <span style="color:red">**Intelligent failover provides uninterrupted service even if one or more of the CDN servers go offline**</span> due to hardware malfunction; the failover can <span style="background-color:#FFFF00">**redistribute the traffic (可以redistribute traffic 到背的server)**</span> to the other operational servers.
- In the event that an entire data center is having technical issues, <span style="background-color:#FFFF00">**Anycast routing transfers the traffic to another available data center (如果一个data center 有问题，transfer 到另外一个)**</span>, ensuring that no users lose access to the website.

#### Data Security - How does a CDN protect data?

Information security is an integral part of a CDN. a CDN can keep a site secured with <span style="color:red">fresh TLS/SSL certificates</span> which will ensure a high standard of <span style="color:red">authentication, encryption, and integrity</span>. Investigate the security concerns surrounding CDNs, and explore what can be done to securely deliver content. Learn about CDN SSL/TLS security


## Data Partitioning

Data partitioning is a technique to <span style="background-color:#FFFF00">**break a big database (DB) into many smaller parts**</span>. It is the process of splitting up a DB/table across multiple machines to improve the manageability, performance, availability, and load balancing of an application. The justification for data partitioning is that, after a certain scale point, <span style="background-color:#FFFF00">**it is cheaper and more feasible to scale horizontally by adding more machines than to grow it vertically by adding beefier servers**</span>.


#### 1. Partitioning Methods#

**Horizontal Partitioning**: In this scheme, we put different rows into different tables. For example, if we store different places in a table, we can decide that locations with ZIP codes less than 10000 are stored in one table and places with ZIP codes greater than 10000 are stored in a separate table. This is also called <span style="background-color:#FFFF00">**range-based Partitioning**</span> as we are storing different ranges of data in separate tables. <span style="background-color:#FFFF00">**Horizontal Partitioning is also known as Data Sharding**</span>.

The key problem with this approach is that if the value whose range is used for Partitioning isn’t chosen carefully, then the partitioning scheme will lead to **unbalanced servers**. In the previous example, splitting locations based on their zip codes assume that <span style="color:red">**places will be evenly distributed across the different zip codes**</span>. This assumption is not valid as there will be a lot of places in a thickly populated area like Manhattan as compared to its suburb cities.


**Vertical Partitioning**: 