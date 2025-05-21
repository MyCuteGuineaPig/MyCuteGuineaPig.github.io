---
layout:     post
title:      "Design Data Intensive Applicaiton"
date:       2025-05-19 20:00:00
author:     "Becks"
header-img: "img/post-bg-rwd.jpg"
catalog:    true
tags:
    - System Design
    - Grook
---

## Distributed Data 

- <span style="background-color:#FFFF00">**Scalability**</span>:If your data volume, read load, or write load grows bigger than a single machine can handle, you can potentially spread the load across multiple machines.
- <span style="background-color:#FFFF00">**Fault tolerance/high availability**</span>: continue working even if one machine (or several machines, or the network, or an entire datacenter) goes down, you can use multiple machines to give you redundancy. When one fails, another one can take over (一个machine off，其他可以用上)
- <span style="background-color:#FFFF00">**Latency**</span>: If you have users around the world, you might want to have servers at various locations worldwide so that each user can be served from a datacenter that is geographically close to them. That avoids the users having to wait for network packets to travel halfway around the world (让用户access离的最近的datacenter)


- <span style="background-color:#FFFF00">**Vertical Scaling**</span>(scaling up): buy more powerful machine，更多的CPU, RAM, Disk
  - **shared-memory** architecture: all the compoents can be treated as a single machine 
  - <span style="color:red">cost grows faster than linearly</span>: a machine with twice as many CPUs, twice as much RAM, and twice as much disk capacity as another typically costs significantly more than twice as much
  - <span style="background-color:#FFFF00">**shared-disk architecture**</span>, which uses several machines with independent CPUs and RAM, but stores data on an array of disks that is shared between the machines, which are connected via a fast network.
    - used for some data warehousing workloads, <span style="color:red">but contention and the overhead of locking limit the scalability</span> of the shared-disk approach
- <span style="background-color:#FFFF00">**horizontal scaling**</span>, scaling out，shared-nothing architectures： 
  - each machine or virtual machine running the database software is called a <span style="background-color:#FFFF00">**node**</span>.
  - <span style="color:red">Each node uses its CPUs, RAM, and disks independently</span>. Any coordination between nodes is done at the <span style="color:red">software level</span>, using a conventional network
  - can use whatever machines have the best price/performance ratio
  - potentially distribute data across multiple geographic regions, and thus reduce latency for users and potentially be able to survive the loss of an entire datacenter


**Replication Versus Partitioning**

- <span style="background-color:#FFFF00">**Replication**</span>: Replication provides redundancy: if some nodes are unavailable, the data can still be served from the remaining nodes
- <span style="background-color:#FFFF00">**Partitioning**</span>: Splitting a big database into smaller subsets called partitions so that different partitions can be assigned to different nodes (also known as sharding)


## Replication

several reasons why you might want to replicate data

- To <span style="background-color:#FFFF00">keep data geographically close to your users</span> (and thus reduce latency)
- To allow the system to continue working even if some of its parts have failed (and thus <span style="background-color:#FFFF00">**increase availability**</span>)
- To scale out the number of machines that can <span style="color:red">serve read queries</span>


3种： single-leader, multi-leader, and leaderless replication


#### Leader and Followers

replica: each nodes that stores a copy of the database 
  
<span style="background-color:#FFFF00">**leader-based aopplication**</span>: 

- <span style="background-color:#FFFF00">**One of the replicas is designated the leader**</span> (also known as master or primary). When clients want to <span style="color:red">write to the database, they must send their requests to the leader</span>, which first writes the new data to its local storage.
- The other replicas are known as <span style="background-color:#FFFF00">**followers**</span> (read replicas, <span style="background-color:#FFFF00">**slaves**</span>, secondaries, or hot standbys).i Whenever the leader writes new data to its local storage, it also sends the data change to all of its followers as part of a replication log or change stream. Each follower takes the log from the leader and updates its local copy of the database accordingly, by applying all writes in the same order as they were processed on the leader
- <span style="background-color:#FFFF00">When a client wants to read from the database, it can query either the leader or any of the followers. However, writes are only accepted on the leader</span> (the followers are read-only from the client’s point of view)  <span style="background-color:#FFFF00">**读可以从leader or follower但写必须是leader**</span>
- A built-in feature of many relational databases, such as PostgreSQL, MySQL, Oracle, SQL Server. Aslo used in some nonrelational database, including MongoDB, RethinkDB and Espresso. 


#### Synchronous Versus Asynchronous Replication

- <span style="background-color:#FFFF00">**synchronous**</span>: the leader
waits until all followers have confirmed that it received the write before reporting success to the user, and before making the write visible to other clients
  - <span style="background-color:#FFFF00">**Advantage**</span>: follower is guaranteed to have an up-to-date copy of the data that is consistent with the leader. If the leader suddenly fails, we can be sure that the data is still available on the follower (<span style="background-color:#FFFF00">**保证all followers 跟leader有一样的data, 如果leader fails, data still available on follower**</span>)
  - Disadvantage: if the synchronous follower doesn’t respond (because it has crashed, or there is a network fault, or for any other reason), the write cannot be processed. The leader must block all writes and wait until the synchronous replica is available again (<span style="background-color:#FFFF00">**如果follower 不respond, leader会被blocker**</span>)
  - <span style="background-color:#FFFF00">**Impractical**</span>: any one node outage would cause the whole system to grind to a halt (<span style="background-color:#FFFF00">**一个follower坏掉，整个system halt**</span>)
- <span style="background-color:#FFFF00">**asynchronous**</span>: the leader sends the message, but <span style="background-color:#FFFF00">**doesn’t wait for a response from the follower**</span>
  - <span style="color:red">**no guarantee of how long it might take**</span>. There are circumstances when followers might fall behind the leader by several minutes or more; for example, if a follower is recovering from a failure, if the system is operating near maximum capacity, or if there are network problems between the nodes.
  - leader-based replication is configured to be completely asynchronous：if the leader fails and is not recoverable, any writes that have not yet been replicated to followers are lost. <span style="background-color:#FFFF00">**如果leader fail，any writes 还没到replicas就丢失了**</span>
  - <span style="background-color:#FFFF00">**Advantage**</span>: leader can continue processing writes, even if all of its followers have fallen behind. <span style="background-color:#FFFF00">**leader 可以一直processing 即使follower failed**</span>
- <span style="background-color:#FFFF00">**semi-synchronous**</span>: 一个node sync 其他node async
  -  If the synchronous follower becomes unavailable or slow, one of the asynchronous followers is made synchronous 当sync的node变慢或者不available, 其他async变成sync
  

#### Setup New Followers

- <span style="color:red">Simply copying data files from one node to another is typically not sufficient: clients are constantly writing to the database, and the data is always in flux</span>, so a standard file copy would see different parts of the database at different points in time. 
- Could make the files on disk consistent by <span style="color:red">locking the database (making it unavailable for writes)</span>, but <span style="color:red">go against our goal of high availability</span>
- process looks like this: 
  - <span style="background-color:#FFFF00">**Take a consistent snapshot of the leader’s database at some point in time—if possible**</span>, without taking a lock on the entire database. Most databases have this feature, as it is also required for backups. In some cases, third-party tools are needed, such as innobackupex for MySQL
  - <span style="background-color:#FFFF00">**Copy the snapshot to the new follower node**</span>
  - The <span style="background-color:#FFFF00">**follower connects to the leader and requests all the data changes**</span> that have happened since the snapshot was taken. This requires that <span style="background-color:#FFFF00">**the snapshot is associated with an exact position in the leader’s replication log**</span>. That position has various names: for example, PostgreSQL calls it the log sequence number, and ySQL calls it the binlog coordinates.
  - When the follower has <span style="background-color:#FFFF00">**processed the backlog of data changes since the snapshot **</span>we say it has <span style="color:red">**caught up**</span>. It can now continue to process data changes from the leader as they happen

#### Follower failure


Being able to reboot individual nodes without downtime is a big advantage for operations and maintenance. Thus, our goal is to keep the system as a whole running despite individual node failures, and to keep the impact of a node outage as small as possible.

- On its local disk, each follower keeps a log of the data changes it has received from the leader. 每一个follower都有来自leader的data changes log. 
- If a follower crashes and is restarted, or if the network between the leader and the follower is temporarily interrupted, the follower can recover quite easily: from its log, it <span style="background-color:#FFFF00">**knows the last transaction that was processed before the fault occurred**</span>. 
- Thus, the follower can connect to the leader and request all the data changes that
occurred during the time when the follower was disconnected. When it has applied
these changes, it has caught up to the leader and can continue receiving a stream of
data changes as before. (<span style="background-color:#FFFF00">**follower知道crash上一个transaction, 去leader那里去要**</span>)


#### Leader failure: Failover

<span style="background-color:#FFFF00">**one of the followers needs to be promoted to be the new leader**</span>, clients need to be reconfigured to send their writes to the new leader, and the other followers need to start consuming data changes from the new leader. This process is called <span style="background-color:#FFFF00">**failover**</span> <span style="background-color:#FFFF00">**一个follower变成leader，client需要reconfigure 发送到新leader 其他follower consume data from new leaders**</span>

failover consists of following steps:

- <span style="background-color:#FFFF00">**Determining that the leader has failed**</span>. Many things could potentially go wrong: crashes, power outages, network issues, and more. <span style="background-color:#FFFF00">**most systems simply use a timeout**</span>: nodes frequently bounce messages back and forth between each other, and <span style="background-color:#FFFF00">**if a node doesn’t respond for some period of time—say, 30 seconds—it is assumed to be dead**</span>. (If the leader is deliberately taken down for planned maintenance, this doesn’t apply.)
- <span style="background-color:#FFFF00">**Choosing a new leader**</span>. This could be done through an **election process** (where the leader is chosen by a majority of the remaining replicas), or a new leader could be appointed by a previously elected controller node. <span style="background-color:#FFFF00">**The best candidate for leadership is usually the replica with the most up-to-date data changes from the old leader (to minimize any data loss)**</span> <span style="background-color:#FFFF00">选一个最up-to-date 来减少data loss</span>
- Reconfiguring the system to use the new leader. <span style="background-color:#FFFF00">**Clients now need to send their write requests to the new leader** </span>. If the old leader comes back, it might still believe that it is the leader, not realizing that the other replicas have forced it to step down. <span style="background-color:#FFFF00">**The system needs to ensure that the old leader becomes a follower and recognizes the new leader**</span>. 需要reconfig 新的leader 让old leader变成follower


<span style="background-color:#FFFF00">**Failover**</span> is fraught with things that can go wrong:

- aysnc: <span style="color:red">new leaeder may not received all writes from old leader before it failed. 如果old leader rejoin after new leader chosen -> conflit</span>. The most common solution for old leader's unreplicated writes to simply be discarded, which violates clients' durability expectation.
  - Discarding writes is especially <span style="color:red">dangerous if other storage systems outside of the database need to be coordinated with the database content</span>. 
  - For example, in one incident at GitHub [13], an out-of-date MySQL follower was promoted to leader. The database used an autoincrementing counter to assign primary keys to new rows, but because the new leader’s counter lagged behind the old leader’s, it reused some primary keys that were previously assigned by the old leader. These primary keys were also used in a Redis store, so the reuse of primary keys resulted in inconsistency between MySQL and Redis, which caused some private data to be disclosed to the wrong users.
- 一些 fault scenarios, it could happen that <span style="background-color:#FFFF00">**two nodes both believe that they are the leader**. </span>This situation is called <span style="background-color:#FFFF00">**split brain**</span>, and it is dangerous: if both leaders accept writes, and there is no process for resolving conflicts, data is likely to be lost or corrupted. As a safety catch, some systems have a mechanism to shut down one node if two leaders are detected. However, if this mechanism is not carefully designed, you can end up with both nodes being shut down  (<span style="background-color:#FFFF00">**两个node 都说他们是leader, 叫做split brain, 最常见的方法是关闭一个node，但有可能两个都关了**</span>)
- What is the right timeout before the leader is declared dead? <span style="background-color:#FFFF00">**A longer timeout means a longer time to recovery in the case where the leader fails**</span>. However, if the timeout is too short, there could be unnecessary failovers. <span style="background-color:#FFFF00">**更长的timeout 表示更久leader fail, 更长recover 如果timeout too short, uncessary failovers**</span>
  - For example, a temporary load spike could cause a node’s response time to increase above the timeout, or a network glitch could cause delayed packets. If the system is already struggling with high load or network problems, an unnecessary failover is likely to make the situation worse, not better
- There are no easy solutions to these problems. For this reason, some operations teams prefer to perform failovers manually, even if the software supports automatic failover 


#### Implementation of Replication Logs

- <span style="background-color:#FFFF00">**Statement-based replication**</span>: In the simplest case, the leader logs every write request (statement) that it executes and sends that statement log to its followers. leader 把每个write request 都发给followers
  - For SQL, every INSERT, UPDATE, or DELETE statement is forwarded to followers, and each follower parses and executes that SQL statement as if it had been received from a client
  - <span style="color:red">**Some problems**</span>:
    - nondeterministic function, such as `NOW()` to get the current date and time or `RAND()` to get a random number, is likely to <span style="background-color:#FFFF00">**generate a different value on each replica**</span> 
    - 一些autoincremetning column depend on the existing data in the database (e.g., UPDATE … WHERE <some condition>), they must be executed in exactly the same order on each replica, or else they may have a different effect. This can be <span style="background-color:#FFFF00">**limiting when there are multiple concurrently executing transactions**</span>. (一些autoincrementing column 必须execute on the same order)
    - Statements that have side effects (e.g., triggers, stored procedures, user-defined functions) may result in different side effects occurring on each replica, unless the side effects are absolutely deterministic
  - Usage: Statement-based replication was used in MySQL before version 5.1. It is still sometimes used today, as it is quite compact, but by default MySQL now switches to <span style="background-color:#FFFF00">**rowbased replication**</span> (discussed shortly) if there is any nondeterminism in a statement. VoltDB uses statement-based replication, and makes it safe by requiring transactionsto be deterministic
- <span style="background-color:#FFFF00">**Write-ahead log (WAL) shipping**</span>: 