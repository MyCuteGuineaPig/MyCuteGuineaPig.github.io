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
  - Log-structured storage or B-tree append -only sequence of bytes containing all writes to the database. 可以exact same log to build replica on another node.
    - besides writing the log to disk, the leader also sends it across the network to its followers. When the follower processes this log, it builds a copy of the exact same data structures as found on the leader
    - <span style="background-color:#FFFF00">**Disadvantage**</span>:<span style="color:red">log describes the data on a very low level</span>
      - **makes replication closely coupled to the storage engine**. If the database changes its storage format, <span style="background-color:#FFFF00">typically not possible to run different versions of the database software on the leader and the followers</span> 如果database change format 不能run 两种在leader和follower
      - <span style="background-color:#FFFF00">**听起来minor implementation, big impact, 如果follower可以有different version than leader, 可以 zero-downtime upgrade of database by first upgrading followers 再performing a failover to 让其中一个follower变成leader. 如果replication protocol does not allow version mismatch -> 这种upgrades 需要downtime**</span>
  - <span style="background-color:#FFFF00">**Logical (row-based) log replication**</span>：use different log formats for replication and for the storage engine, which<span style="background-color:#FFFF00">**allows the replication log to be decoupled from the storage engine internals**. </span>This kind of replication log is called a <span style="background-color:#FFFF00">**logical log**</span>, to distinguish it from the storage engine’s (physical) data representation
    - A logical log for a relational database is usually a **sequence of records describing writes to database tables** at the granularity of a row:
      - For an inserted row, the log contains the new values of all columns. 
      - For a deleted row, the log contains enough information to uniquely identify the row that was deleted. Typically this would be the primary key, but if there is no primary key on the table, the old values of all columns need to be logged
      - For an updated row, the log contains enough information to uniquely identify the updated row, and the new values of all columns (or at least the new values of all columns that changed).
      - a logical log is decoupled from the storage engine internals, <span style="background-color:#FFFF00">**it can more easily be kept backward compatible**</span>
      - easier for external applications to parse
- <span style="background-color:#FFFF00">**Trigger-based replication**</span>：上面都没有involve application code, 但有时候want only a subset of data, replicate from one kind of database to another -> move replication up to the application layer.
  - Some tools, such as Oracle GoldenGate [19], can make data changes available to an application by reading the database log
  - A trigger lets you register custom application code that is automatically executed when a data change (write transaction) occurs in a database system.
    - <span style="background-color:#FFFF00">**log this change into a separate table**</span>. read by external process. External process can apply 任何application logic and replicate the data change to another system.
  - has overheads than other method, 更prone to bug


#### Problems with Replication Lag

<span style="background-color:#FFFF00">**read-scaling architecture**</span>: increase the capacity for serving read-only requests simply by adding more followers. 只work for async, 如果sync, 一个follower failure or network outage -> make entire system unavailable for writing.  -> more nodes, 越unreliable

<span style="background-color:#FFFF00">**如果application read from aync follower, 也许outdated information. inconsistencies in the database**</span>. if you stop writing to the database and wait a while, the followers will eventually catch up and become consistent with the leader. the delay between a write happening on the leader and being reflected on a follower—<span style="background-color:#FFFF00">**the replication lag**</span>. 大概几秒not noticable


#### Reading Your Own Writes

- async, if the user views the data shortly after making a write, the new data may not yet have reached the replica. 
  -  we need <span style="color:red">*read-after-write consistency*</span>, also known as <span style="background-color:#FFFF00">**read-your-writes consistency**</span>. This is a guarantee that <span style="background-color:#FFFF00">**if the user reloads the page, they will always see any updates they submitted themselves**. </span>It makes no promises about other users.
  

How to implement? 

- When reading something that the user may have modified, read it from the
leader; otherwise, read it from a follower. 需要一定机制know whether modified 
  - 比如, user profile on social network, 只能owner 修改，a simple rule: always rad the user's own profile from the leader, any other users' profile from a follower
  - make all reads from the leader. 比如 monitor the replication lag on followers and prevent queries on any follower that is more than one minute behind the reader. 
  - <span style="color:red">client can remember the timestamp of most recent write</span> - system can ensure that the replica serving any reads fro that user reflects updates at tleast until that timestamp. 如果replica not up to date, <span style="color:red">either the read can be handled by another replica or query can wait unitl the replica has caught up</span>. timestamp 可以是logical timestamp or actual system clock
  - 如果 replicas distributed across multiple datacenters, additonal complexity, <span style="background-color:#FFFF00">**Any request that needs to be served by the leader must routed to the datacenter that contains the leader**</span>


**cross-device read-after-write consistency**: 比如desktop write, view it on another device.<span style="background-color:#FFFF00">**If your replicas are distributed across different datacenters, there is no guarantee that connections from different devices will be routed to the same datacenter**</span>. 如果replica 是不同的datacenter, 不能保证connection from different device route to the same datacenter


<span style="background-color:#FFFF00">**Monotonic reads**</span>： they will not read older data after having previously read newer data.


<span style="background-color:#FFFF00">**consistent prefix reads**</span>. This guarantee says that if a sequence of writes happens in a certain order, then anyone reading those writes will see them appear in the same order. <span style="background-color:#FFFF00">One solution is to make sure that any writes that are causally related to each other are written to the same partition</span>


#### Multiple-Leader Replication 

- allow more than one node to accept writes (<span style="background-color:#FFFF00">**multi-leader configuration**</span>, also known as <span style="background-color:#FFFF00">**master–master**</span> or a<span style="background-color:#FFFF00">**ctive/active replication**</span>)
  - Replication still happens in the same way: <span style="background-color:#FFFF00">**each node that processes a write must forward that data change to all the other nodes**</span>.
  - Multi-datacenter operation. 非常rare只有一个datacenter. have a leader in each datacenter
    - Within each datacenter, regular leader-follower replication is used; between datacenters, <span style="background-color:#FFFF00">**each datacenter’s leader replicates its changes to the leaders in other datacenters**</span>.



- <span style="background-color:#FFFF00">**Performance**</span>:In a single-leader configuration, every write must go over the internet to the datacenter with the leader. This can add significant latency to writes and might contravene the purpose of having multiple datacenters in the first place. In a multi-leader configuration, <span style="background-color:#FFFF00">**every write can be processed in the local datacenter and is replicated asynchronously to the other datacenters**</span>. Thus, the inter-datacenter <span style="background-color:#FFFF00">**network delay is hidden from users**</span>, which means the perceived performance may be better 
- <span style="background-color:#FFFF00">**Tolerance of datacenter outages**</span>:each datacenter can continue operating independently of the others, and replication catches up when the failed datacenter comes back online (<span style="background-color:#FFFF00">**每个datacenter 都是相互独立的，replication 可以catches up when failed, datacenter可以come back oneline**</span>)
- **Tolerance of network problems**: 
  - Traffic between datacenters usually goes over the public internet, which may be less reliable than the local network within a datacenter
  - A single-leader configuration is very sensitive to problems in this inter-datacenter link. (<span style="background-color:#FFFF00">**Single-leader 是非常容易受到inter-datacenter link影响的, 因为write sync over this link**</span>)
  - A multi-leader configuration with <span style="background-color:#FFFF00">**asynchronous**</span> replication can usually tolerate network problems better: **a temporary network interruption does not prevent writes being processed** (<span style="background-color:#FFFF00">暂时的network interruption 不会影响writes being processed</span>)
- Multiple leader database 例子：  Tungsten Replicator for MySQL [26], BDR for PostgreSQL [27], and GoldenGate for Oracle
- disadvantage:  the same data may be concurrently modified in two different datacenters, and those <span style="background-color:#FFFF00">**write conflicts must be resolved**</span>
- <span style="background-color:#FFFF00">**autoincrementing keys, triggers, and integrity constraints can be problematic**</span>. For this reason, multi-leader replication is often considered <span style="color:red">**dangerous**</span> territory that should be avoided if possible 


需要multi-leader replication if you have an application that needs to continue to work while it is disconnected from the internet. 比如电脑，手机操作是sync的， <span style="background-color:#FFFF00">**every device has a local database that acts as a leader**</span> -> extremely unreliable

Google doc 多个人editing. <span style="color:red">changes are instantly applied to their local replica (the state of the document in their web browser or client application) and asynchronously replicated to the server and any other users who are editing the same document</span>. If you want to guarantee that there will be no editing conflicts, the application must obtain a lock on the document before a user can edit it. If another user wants to edit the same document, they first have to wait until the first user has committed their changes and released the lock
 

#### Handling Write Conflicts

![](/img/post/ddia/5-7.png)


- In a single-leader database, the second writer will either block and wait for the first write to complete, or abort the second write transaction, forcing the user to retry the write. (第二个会等第一个完成)
- in a multi-leader setup, both writes are successful, and the conflict is only detected asynchronously at some later point in time (不会有conflict)
- avoid conflict 最简单的办法: all writes for a particular record go through the same leader, then conflicts cannot occur
- 有时候要change designated leader 因为datacenter failed, 需要reroute traffic to another datacenter 或者 a user move to a different location 现在close to a different datacenter. 这种情况, have to deal with the possibility of concurrent writes on different leaders

几种方式resolve conflict

- Give each write a unique ID, 最高ID as the winner
- If timestamp is used, last write wins (**<span style="background-color:#FFFF00">可能会有data loss</span>**)
- <span style="background-color:#FFFF00">给每个replica 一个ID, writes that originated at a higher numbered replica always take precendence over writes that originated at a lower-numbered replica. => implies data loss
</span>
- Somehow merge the values together—e.g., order them alphabetically and then concatenate them
- Record the conflict in an explicit data structure that preserves all information,and write application code that resolves the conflict at some later time


**Custom conflict resolution logic**

- <span style="background-color:#FFFF00">**On write**</span>: detects a conflict in the log of replicated changes, it calls the conflict handler. This handler typically cannot prompt a user—it <span style="background-color:#FFFF00">**runs in a background process and it must execute quickly**</span>
- <span style="background-color:#FFFF00">**On read**</span>: The application may <span style="background-color:#FFFF00">**prompt the user or automatically resolve the conflict**</span>, and write the result back to the database.比如 CouchDB

#### Multi-Leader Replication Topologies

![](/img/post/ddia/5-8.png)

- <span style="background-color:#FFFF00">**all-to-all**</span>: every leader sends its writes to every other leader. 
  - <span style="background-color:#FFFF00">**与star和circular相比 没有single points of failure**</span> 
  - 但也有问题， ome network links may be faster than others (e.g., due to network congestion), with the result that some replication messages may “overtake” others, as illustrated in 图。leader 2 收到writes in different order 
    - **Consistent Prefix Reads**: 需要make sure insert first then update. Simply use timestamp 不行，因为clock cannot be trusted to be sufficiently in sync to correctly order these events at leaders
- <span style="background-color:#FFFF00">**circular topology**</span> [34], in which each node receives writes from one node and forwards those writes (plus any writes of its own) to one other node.
  - To prevent infinite replication loops, <span style="background-color:#FFFF00">**each node is given a unique identifier**</span>, and in the replication log, each write is tagged with the identifiers of all the nodes it has passed through
- <span style="background-color:#FFFF00">**a star**</span>:v one designated root node forwards writes to all of the other nodes

![](/img/post/ddia/5-9.png)

A problem with circular and star topologies is that if just one node fails, it can interrupt the flow of replication messages between other nodes, causing them to be unable to communicate until the node is fixed (<span style="background-color:#FFFF00">**对于circular 和star, 一个node 坏了可能影响着其他的node**</span>)。 The topology could be reconfigured to work around the failed node, but in most deployments such reconfiguration would have to be done manually


#### Leaderless Replication

Dynamo system (Amazon). Riak, Cassandra, and Voldemort are open source datastores with leaderless replication models inspired by Dynamo, so this kind of database is also known as Dynamo-style

In some leaderless implementations, the client directly sends its writes to several replicas, while in others, a coordinator node does this on behalf of the client. However, unlike a leader database, that coordinator does not enforce a particular ordering of writes (<span style="background-color:#FFFF00">**client 直接发给replicas, coordinator node 做这个behalf of client, 不像leader database, coordinator 不会enforce a particular ordering of writes**</span>)


<span style="background-color:#FFFF00">**failover**</span>:

- leader-based configure, 想processing writes, perform a failover
- leaderless configuration. failover 不存在

![](/img/post/ddia/5-10.png)

replica 3 miss the write. <span style="color:red">**当replica 3 back online, 可能有stale data**</span>. read requests are also sent to several nodes in parallel. The client may get different responses from different nodes; i.e., the up-to-date value from one node and a stale value from another. <span style="background-color:#FFFF00">**Version numbers**</span> are used to determine which value is newer.   


**Read repair and anti-entropy**

- <span style="background-color:#FFFF00">**Read repair**</span>: 比如user read version 1 and 3 from replica 1 and 2. see replica 3 has a stale value, and writes the newer value back to that replica.
- <span style="background-color:#FFFF00">**Anti-entropy process**</span>: some datastore have a background process 持续看differences in the data between replicas and copies any missing data from one replica to another.
  - this anti-entropy process does not copy writes in any particular order, and there maybe a significant delay before data is copied 


If there are **n replicas**, every write must be confirmed by **w nodes** to be considered successful, and we must query **at least r nodes** for each read. (In our example, n = 3, w = 2, r = 2.) As long as <span style="background-color:#FFFF00">**w + r > n**</span>, we expect to get an up-to-date value when reading, because at least one of the r nodes we’re reading from must be up to date. Reads and writes that obey these <span style="background-color:#FFFF00">**r and w values are called quorum reads and writes**</span>. You can think of r and w as the minimum number of votes required for the read or write to be valid.

In Dynamo-style databases, <span style="background-color:#FFFF00">**the parameters n, w, and r are typically configurable**</span>. A common choice is to make n an odd number (typically 3 or 5) and to set w = r = (n + 1) / 2 (rounded up)

For example, a workload with few writes and many reads may benefit from setting w = n and r = 1. This<span style="color:red">** makes reads faster, but has the disadvantage that just one failed node causes all database writes to fail 设置w, n ,r 都是1， 可以make read faster 但是single point of failure**</span>.

<span style="background-color:#FFFF00">**w+r>n, exepect every read to return the most recent value written for a key, because the set of nodes to which you’ve written and the set of nodes from which you’ve read must overlap 因read 和write 一定会重合一些node**</span>

- If w < n, we can still process writes if a node is unavailable.
- If r < n, we can still process reads if a node is unavailable.
- With n = 3, w = 2, r = 2 we can tolerate one unavailable node.
- With n = 5, w = 3, r = 3 we can tolerate two unavailable nodes

![](/img/post/ddia/5-11.png)


<span style="background-color:#FFFF00">**If fewer than the required w or r nodes are available, writes or reads return an error**</span>. A node could be unavailable for many reasons: because the node is down (crashed, powered down), due to an error executing the operation (can’t write because the disk is full), due to a network interruption between the client and the node, or for any number of other reasons

Often, <span style="background-color:#FFFF00">**r and w are chosen to be a majority**</span> (more than n/2) of nodes, because that ensures w + r > n while still tolerating up to n/2 node failures.

may also <span style="background-color:#FFFF00">**set w and r to smaller numbers, so that w + r ≤ n**</span> (i.e., the quorum condition is not satisfied). In this case, reads and writes will still be sent to n nodes, <span style="background-color:#FFFF00">**but a smaller number of successful responses is required for the operation to succeed**</span>

- <span style="background-color:#FFFF00">**return stale value**</span>
- <span style="background-color:#FFFF00">**allow lower latency and higher availability**</span>
- if there is a network interruption and many replicas become unreachable, there’s a higher chance that you can continue processing reads and writes. Only after the number of reachable replicas falls below w or r does the database become unavailable for writing or reading, respectively

但是 即使 w + r > n 也有edge cases where stale values are returned 

- If a <span style="background-color:#FFFF00">**sloppy quorum**</span> is used, the w writes may end up on different nodes than the r reads, so there is no longer a guaranteed overlap between the r nodes and the w nodes
- If two writes occur concurrently, it is not clear which one happened first. In this case, the only safe solution is to merge the concurrent writes, If a winner is picked based on a timestamp (last write wins), writes can be lost due to clock skew
- If a write happens concurrently with a read, the write may be reflected on only some of the replicas. In this case, it’s undetermined whether the read returns the old or the new value
- If a write succeeded on some replicas but failed on others (for example because the disks on some nodes are full), and overall succeeded on fewer than w replicas, it is not rolled back on the replicas where it succeeded. This means that if a write was reported as failed, subsequent reads may or may not return the value from that write
- If a node carrying a new value fails, and its data is restored from a replica carrying an old value, the number of replicas storing the new value may fall below w, breaking the quorum condition 一个node fail 从old replica 恢复，可能没有新的数据
- Even if everything is working correctly, there are edge cases in which you can get unlucky with the timing


Dynamo-style databases are generally optimized for use cases that can tolerate eventual consistency.

#### Monitoring staleness

- because writes are applied to the leader and to followers in the same order, and <span style="background-color:#FFFF00">**each node has a position in the replication log**</span>. By subtracting a follower’s current position from the leader’s current position, you can measure the amount of replication lag.
- <span style="background-color:#FFFF00">**Leaderless Replication**</span>, there is **no fixed order** in which writes are applied, which makes monitoring more difficult.
  - 如果only use only uses read repair (no anti-entropy), there is no limit to how old a value might be f a value is only infrequently read, the value returned by a stale replica may be ancient.

#### Sloppy Quorums and Hinted Handoff

- 如果有internet issue, a client that is cut off from the database nodes, they might aslo dead. 很有可能少于w or r个 reachable nodes remain, client no longer reach a quorum. 
- <span style="background-color:#FFFF00">**Sloppy quorum**</span>: accept writes anyway, write them to some nodes that are reachable but aren't among the n nodes on which the value usually lives
  - writes and reads still require w and r successful responses, but those may include nodes that are not among the designated n “home” nodes for a value. 比如你家被锁了，在邻居家暂住一下
  - Once the network interruption is fixed, any writes that one node temporarily accepted on behalf of another node are sent to the appropriate “home” nodes 当internet 恢复，the node tempoarily accepted write 被发到原来属于的nodes
- Sloppy quorums are particularly useful for <span style="background-color:#FFFF00">**increasing write availability**</span>: as long as any w nodes are available, the database can accept writes. However, this means that even when w + r > n, you cannot be sure to read the latest value for a key, because the latest value may have been temporarily written to some nodes outside of n
- **a sloppy quorum actually isn’t a quorum at all in the traditional sense**. It’s only an assurance of <span style="background-color:#FFFF00">**durability**</span>, namely that the data is stored on w nodes somewhere. There is no guarantee that a read of r nodes will see it until the hinted handoff has completed


#### Detecting Concurrent Writes

events may arrive in a different order at different nodes, due to variable network delays and partial failures

![](/img/post/ddia/5-12.png)


- Node 1 receives the write from A, but never receives the write from B due to a
transient outage.
- Node 2 first receives the write from A, then the write from B.
- Node 3 first receives the write from B, then the write from A.

If each node simply overwrote the value for a key whenever it received a write request from a client, the nodes would become permanently inconsistent. 如果一个node 只是简单的接受一个node, node变得inconsistent

**Last write wins (discarding concurrent writes)** conflict resolution method in Cassandra

- One approach for achieving eventual convergence is to declare that each replica <span style="background-color:#FFFF00">need only store the most “recent” valu</span>e and allow “older” values to be overwritten and discarded
  - 只要有一种方式决定什么是more recent, every writes is eventually copied to every replica, the replicas will 最终converge to the same value 
- at the cost of <span style="background-color:#FFFF00">**durability**</span>: 
  - several concurrent writes to the same key, even if they were all reported as successful to the client (because they were written to w replicas), only one of the writes (多个write，可能都返回给client成功，最后很有可能只有一个成功)
will survive and the others will be silently discarded
- LWW may drop writes that are not concurrent
- only safe way of using a database with LWW is to ensure that <span style="background-color:#FFFF00">**a key is only written once and thereafter treated as immutable**</span>, thus <span style="background-color:#FFFF00">**avoiding any concurrent updates to the same key**</span>

Concurrent 表示没有causal dependency. casual dependency 比如 set value = 1, 第二个operation是 value = value + 1, 如果没有value 就不能到第二步

whenever you have two operations A and B, there are three possibilities: either A happened before B, or B happened before A, or A and B are concurrent.

For defining concurrency, exact time doesn’t matter. we simply call two operations concurrent if they are both unaware of each other, regardless of the physical time at which they occurred


**algorithm**: use version number

- The server maintains a version number for every key, increments the version number every time that key is written, and stores the new version number along with the value written.
- When a client reads a key, the server returns all values that have not been over‐ written, as well as the latest version number. A client must read a key before writing. client必须 reads a key before writing   
- When a client writes a key, it must include the version number from the prior read, and it must merge together all values that it received in the prior read. (The response from a write request can be like a read, returning all current values, which allows us to chain several writes like in the shopping cart example.) 当client writes a key, <span style="background-color:#FFFF00">**必须include the version number 从之前的read, 必须merge all values that it received by prior read**</span>
- When the server receives a write with a particular version number, it can over‐ write all values with that version number or below (since it knows that they have been merged into the new value), but it must keep all values with a higher version number (because those values are concurrent with the incoming write). <span style="background-color:#FFFF00">**当server 收到一个write with version, 必须overwrite all values <= 它的version number, keep all values with a higher version number**</span>
- 如果是remove, take the union may not yield the right result.
  - an item cannot simply be deleted from the database when it is removed; instead the system must leave a marker with an appropriate version number to indicate that the item has been removed when merging siblings. Such a <span style="background-color:#FFFF00">**deletion marke**</span>r is known as a <span style="background-color:#FFFF00">**tombstone**</span>
- <span style="background-color:#FFFF00">**use a version number per replica as well as per key**</span>. Each replica increments its own version number when processing a write, and also keeps track of the version numbers it has seen from each of the other replicas. <span style="background-color:#FFFF00">**每一个replica的每个key 都有一个version number**</span>
- The version vector allows the database to distinguish between overwrites and concurrent writes. <span style="background-color:#FFFF00">**version vector 用来区分是overwrites 还是concurrent writes**</span>
  - 确保了safe to read from one replica and subsequently write back to another replica

## Partition

partitions are defined in such a way that each piece of data (each record, row, or document) belongs to exactly one partition. 

Main reason for wanting to partition data is <span style="background-color:#FFFF00">**scalability**</span>. Partition usually combine with replication. 表示即使each record belongs to exactly one partition, it may still be <span style="background-color:#FFFF00">**stored on several different nodes for fault tolerance**</span>. 

A node may store than one partition. 如果leader-follower replication model is used, combination of parittiong and replication can look like below. Each partitions's leader is assigned to one node, and its followers are assigned to other nodes. 每个node maybe the leader for some partitions and a follower for other parition

![](/img/post/ddia/6-1.png)

<span style="background-color:#FFFF00">**skewed**</span>: if partition is unfair, some partitions have more data or queries than others. Skewed 出现makes partitiong much less effective. A partition with disproportionately high load is called a <span style="background-color:#FFFF00">**hot spot**</span>.

避免hot spot最简单的方法就是assign records to nodes randomly, <span style="background-color:#FFFF00">**有disadvantage**</span>: 当read a particular item, have no way of knowing which node it is on, 必须query all nodes in parallel. 


#### Partitioning by Key Range

assign a continuous range of keys (从最小到最大) to each partition, 像百科全书一样. If you know the boundaries bewteen ranges, can easily determin which partion contains a given key. <span style="color:purple">如果知道which partition is assigned to which node, 可以make your request directly to the appropriate node</span>.