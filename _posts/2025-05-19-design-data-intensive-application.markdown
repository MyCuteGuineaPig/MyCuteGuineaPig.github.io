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

![](/img/post/ddia/6-2.png)

simple having one volume per two letters of alphabet 导致 some volumes being much bigger than others. 为了distribute the data evenly, the parition boundaries need to adapt to the data.  

the downside of key range partitioning is that <span style="background-color:#FFFF00">**certain access patterns can
lead to hot spots**</span>


#### Partitioning by Hash of Key

Because of the risk of skew and hot spots, many distributed datastores 使用a hash function to determine the partition for a given key. <span style="background-color:#FFFF00">**A good hash function takes skewed data and make it uniformly distributed**</span>. 


Once you have a suitable hash function for keys, you can assign each partition a range of hashes (rather than a range of keys), and every key whose hash falls within a partition’s range will be stored in that partition.

![](/img/post/ddia/6-3.png)



**Consistent Hashing**:  is a way of <span style="background-color:#FFFF00">**evenly distributing load**</span> across an internet-wide system of caches such as a <span style="background-color:#FFFF00">**content delivery network (CDN)**</span>. It randomly chosen partition boundaries to avoid the need for central control or distributed consensus. (<span style="background-color:#FFFF00">**consistent here has nothing to do with replica consistency or ACID consistency**</span>). Describes a particular approach to rebalancing.


Downside of hash: lose a nice property of key-range partition: do efficent range queries. <span style="color:purple">**Keys that were once adjacent are now scattered across all the partitions. sort order is lost**</span>.

Cassandra achieves a compromise between the two partitioning strategies. <span style="background-color:#FFFF00"> Cassandra can be declared with a **compound primary key** consisting of several columns</span>. Only the <span style="background-color:#FFFF00">**first part of that key is hashed to determine the partition**</span>, but the other columns are <span style="color:red">used as a concatenated index for sorting the data in Cassandra’s SSTables</span>. A query therefore cannot search for a range of values within the first column of a compound key, but if it specifies a fixed value for the first column, it can perform an efficient range scan over the other columns of the key. <span style="background-color:#FFFF00">**第一个key决定partition, 第二个key 决定sorted order**</span>

比如Cassandra, primary key for update `(user_id, update_timestamp)`. 可以efficiently retrieve all updates made by a particular user within some time interval, sorted by timestamp. 不同的user may be stored on different partitions, 但是within each user, updates are stored ordered by timestamp on a single partition 

#### Skewed Workloads and Relieving Hot Spots

- 比如名人的twitter, application responsibility to reduce the skew. 比如一个key too hot, add a random number to hte beginning or end of the key. 比如两个random number 可以writs to the key evenly across 1000 different keys
- Read has addtional work. have to read the data from all 100 keys and combine it -> <span style="background-color:#FFFF00">需要additional bookkeeping. Only make sense to append the random number for small number of hot keys. 对于大多数keys with low write throughput, 将会是overhead</span>.
- need trade-off for applicaiton

**Partition by document 是 each partition holds a subset of the entire collection of documents. Partition by term (global indexing) 表示index  itself is parititoned**


#### Partitioning and Secondary Indexes

A secondary index usually doesn’t identify a record uniquely but rather is a way of <span style="background-color:#FFFF00">**searching for occurrences of a particular value**</span>

<span style="background-color:#FFFF00">**The problem with secondary indexes is that they don’t map neatly to partitions.**</span> There are two main approaches to partitioning a database with secondary indexes: <span style="background-color:#FFFF00">**document-based partitioning**</span> and <span style="background-color:#FFFF00">**term-based partitioning**</span>

#### Partitioning Secondary Indexes by Document

Each listing has a unique ID—call it the <span style="background-color:#FFFF00">**document ID**</span>—and you partition the database by the document ID

![](/img/post/ddia/6-4.png)


However, reading from a document-partitioned index requires care: unless you have done something special with the document IDs, there is no reason why all the cars with a particular color or a particular make would be in the same partition. In Figure 6-4, red cars appear in both partition 0 and partition 1. Thus, <span style="color:purple">if you want to search for red cars, you need to send the query to all partitions, and combine all the results you get back</span>


querying a partitioned database is sometimes known as <span style="background-color:#FFFF00">**scatter/gather**</span>, and it can make read queries on secondary indexes quite expensive.  Even if you query the partitions in parallel, scatter/gather is prone to tail latency amplification. Q<span style="background-color:#FFFF00">**uerying partition database是expensive and latency**</span>


<span style="color:red">it is widely used</span>: MongoDB, Riak, Cassandra, Elasticsearch, SolrCloud, and VoltDB all use document-partitioned secondary indexes


#### Partitioning Secondary Indexes by Term

Rather than each partition having its own secondary index (a local index), we can construct a global index that covers data in all partitions. 不能只存一个node, <span style="background-color:#FFFF00">**global index 必须也是partitioned, can be partitioned differently from the primary key index**</span>

![](/img/post/ddia/6-5.png)

letter a-r 是partition 0, letter s-z 是 partition 1. called term-partitioned, because the term we’re looking for <span style="background-color:#FFFF00">**determines the partition of the index**</span>

<span style="background-color:#FFFF00">**Partitioning by the term itself can be useful for range scans**</span> (e.g., on a numeric property, such as the asking price of the car), whereas <span style="background-color:#FFFF00">**partitioning on a hash of the term gives a more even distribution of load**</span>. <span style="background-color:#FFFF00">**partition by term itself 用于range scans, partition on hash 给一个更even distribution of load**</span>

<span style="background-color:#FFFF00">**Advantage**</span>:  over a document-partitioned, <span style="background-color:#FFFF00">**make reads more efficient**</span> (rather than doing scatter/gather over all partitions, <span style="color:purple">**a client only needs to make a request to the partition containing the term that it wants**</span>) 

<span style="background-color:#FFFF00">**Downside**</span>: slower and complicated, a write to single document may affect multiple partitions of the index ((every term in the document might be on a different partition, on a different node) <span style="color:red">**因为每个term 可能在不同的partition**</span>


In an ideal world, the index would always be up to date, and every document written to the database would immediately be reflected in the index. However, in a term- partitioned index, that <span style="background-color:#FFFF00">**would require a distributed transaction across all partitions affected by a write**</span>, which is not supported in all databases 


<span style="background-color:#FFFF00">** updates to global secondary indexes are often asynchronous**</span>

#### Rebalancing Partitions

Over time, things change in a database:

- The query throughput increases, so you want to add more CPUs to handle the load.
- The dataset size increases, so you want to add more disks and RAM to store it.
- A machine fails, and other machines need to take over the failed machine’s responsibilities

<span style="background-color:#FFFF00">**moving load from one node in the cluster to another is called rebalancing**</span>

rebalancing 需要 meet some minimum requirements:

- After rebalancing, the load (data storage, read and write requests) should be shared fairly between the nodes in the cluster. <span style="background-color:#FFFF00">**load需要平均分配**</span>
- While rebalancing is happening, the database should continue accepting reads and writes. <span style="background-color:#FFFF00">**当rebalance发生, database 需要接受reads和writes**</span>
- No more data than necessary should be moved between nodes, to make rebalancing fast and to minimize the network and disk I/O load. <span style="background-color:#FFFF00">**只move necessary data，不多move**</span>

#### Strategies for Rebalancing

**How not to do it: hash mod N**

为什么不用 hash(key) mode 10 return a number between 0 and 9 因为number of nodes N changes, most of key 需要move from one node to another. 


**Fixed number of partitions**

![](/img/post/ddia/6-6.png)

If a node is added to the cluster, the new node can steal a few partitions from
every existing node until partitions are fairly distributed once again. the same happens in reverse

The only thing that changes is the assignment of partitions to nodes.<span style="background-color:#FFFF00">** This change of assignment is not immediate**</span>— it takes some time to transfer a large amount of data over the network. 

the number of partitions is usually fixed when the database is first set up and not changed afterward. 

Choosing the right number of partitions is difficult if the total size of the dataset is highly variable (<span style="background-color:#FFFF00">**选择对的partition 数是很困难的如果total size of the dataset 是highly variable**</span>) Since each partition contains a fixed fraction of the total data, the size of each partition grows proportionally to the total amount of data in the cluster. If partitions are very large, rebalancing and recovery from node failures become expensive. But if partitions are too small, they incur too much overhead. 如<span style="background-color:#FFFF00">**果partition 太大, rebalance and recovery from node failure 是非常expensive, 如果partition 太小， incur too much overhead**</span>. "just right,” neither too big nor too small, which can be hard to achieve if the number of partitions is fixed but the dataset size varies.


#### Dynamic partitioning

- if you got the boundaries wrong, you could end up with all of the data in one partition and all of the other partitions empty. Reconfiguring the partition boundaries manually would be very tedious
  - For that reason, key range–partitioned databases such as HBase and RethinkDB create partitions dynamically
- When a partition grows to exceed a configured size (on **HBase**, the default is 10 GB), it is split into two partitions so that approximately half of the data ends up on each side of the split <span style="color:purple">**超过configured size, 自动split成两个partitions **</span>。
- 同理，如果lots of data deleted, a partition shrinks below some threshold, it can be merged with an adjacent partition 
- 每个partition assigned to one node, and each node 可以handle 多个partitions. After a large partition has been split, <span style="color:red">**one of its two halves can be transferred to another node in order to balance the load**</span>. In the case of HBase, the transfer of partition files happens through <span style="background-color:#FFFF00">**HDFS, the underlying distributed filesystem**</span>
- <span style="background-color:#FFFF00">**Advantage: the number of partitions adapts to the total data volume**</span>
  - If there is only a small amount of data, a small number of parti‐
tions is sufficient, so overheads are small; if there is a huge amount of data, the size of each individual partition is limited to a configurable maximum
- empty database start with single partition. 因为没有priori information to draw boundary
  - until it hits the point at which the first partition is split—all writes have to be processed by a single node while the other nodes sit idle
  - To mitigate this issue, **HBase** and **MongoDB** allow an initial set of partitions to be configured on an empty database (this is called <span style="background-color:#FFFF00">**pre-splitting 有一个初始的partition**</span>). In the case of key-range partitioning, <span style="color:purple">pre-splitting requires that you already know what the key distribution is going to look like</span>
- <span style="background-color:#FFFF00">**适用于 key range–partitioned data, 和 hash-partitioned data**</span>


#### Partitioning proportionally to nodes

With <span style="background-color:#FFFF00">**dynamic partitioning**</span>, the number of partitions is proportional to the size of the dataset, since the splitting and merging processes keep the size of each partition <span style="color:red">between some fixed minimum and maximum</span>.  On the other hand, with <span style="background-color:#FFFF00">**a fixed number of partitions**</span>, the size of each partition is proportional to the size of the dataset. Inboth of these cases, the number of partitions is independent of the number of nodes. (<span style="background-color:#FFFF00">**两种情况，number of partitions 都独立于number of nodes**</span>)


A third option, used by **Cassandra** and **Ketama**, is to make the number of partitions proportional to the number of nodes—in other words, to have a fixed number of partitions per node  (<span style="background-color:#FFFF00">**让partition数量与node 成比例**</span>) The size of each partition grows proportionally to the dataset size while the number of nodes remains unchanged, but when you increase the number of nodes, the partitions become smaller again. <span style="background-color:#FFFF00">**当dataset size增加，partition 含有数据增加，当增加number nodes, -> partition 数量减少**</span>。 Since a larger data volume generally <span style="background-color:#FFFF00">**requires a larger number of nodes **</span>to store, this approach also <span style="background-color:#FFFF00">**keeps the size of each partition fairly stable**</span>


当a new node join the cluster, randomly choose a fixed number of exisiting partitions to split, 之后take ownership of one half of each of those split partitions while leave the other half of each partition in place.  <span style="color:red">**The randomization can produce unfair splits**</span>, but when averaged over a larger number of partitions, the new node ends up taking a fair share of the load from the existing nodes. Cassandra 3.0 introduced an alternative rebalancing algorithm that avoids unfair splits 新的node 进来会有unfair spilt, Cassandra 3,0 介入解决了unfair split


#### Operations: Automatic or Manual Rebalancing

Fully manual: the assignment of partitions to nodes is explicitly con‐
figured by an administrator, and only changes when the administrator explicitly
reconfigures it. 

For example, Couchbase, Riak, and Voldemort generate a suggested partition assignment automatically, but require an administrator to commit it before it takes effect.


Fully automated rebalancing

- <span style="background-color:#FFFF00">**convenient, less operational work for normal maintenance**</span>. 
- 但是不可预测. Rebalancing 是 expensive operation, 因为需要rerouting requests and moving a large amount of data from one node to another. 
  - 如果不是done carefully, overload the network or the nodes, and harm perforamnce of other request while rebalancing in process
- can be <span style="background-color:#FFFF00">**dangerous in combination with automatic failure detection**</span>. 比如 one node is overloaded and is temporarily slow to respond to requests. The other nodes conclude that the overloaded node is dead, and automatically rebalance the cluster to move load away from it -> <span style="background-color:#FFFF00">**puts additional load on the overloaded node**</span>, other nodes, and the <span style="background-color:#FFFF00">**network—making the situation worse and potentially causing a cascading failure**</span>
- can be a good thing to have a human in the loop for rebalancing. It’s slower than a fully automatic process, but it can help prevent operational surprises


#### Request Routing

比如want to read or write the key, which IP address and port number do I need to connect to. -> a problem called <span style="background-color:#FFFF00">**service discovery**</span>. 任何software that accessiable over a network has this problem 特别是aim for <span style="background-color:#FFFF00">**high availability**</span>. 

on high level, a few different approaches to this problem

- <span style="background-color:#FFFF00">**Allow clients to contact any node**</span> (e.g., via a <span style="background-color:#FFFF00">**round-robin load balancer**</span>). If that <span style="background-color:#FFFF00">**node coincidentally owns the partition to which the request applies**</span>, it can handle the request directly; otherwise, it <span style="color:purple">**forwards the request**</span> to the appropriate node, receives the reply, and passes the reply along to the client.
- Send all requests <span style="background-color:#FFFF00">**from clients to a routing tier first**</span>, which determines the node that should handle each request and forwards it accordingly. This routing tier does not itself handle any requests; it only acts as a <span style="background-color:#FFFF00">**partition-aware load balancer**</span>. 都先发到routing tier
- Require that clients be aware of the partitioning and the assignment of partitions to nodes. In this case, <span style="background-color:#FFFF00">**a client can connect directly to the appropriate node**</span>, without any intermediary.

the key problem is: how does the component making the routing decision (which may be one of the nodes, or the routing tier, or the client) learn about changes in the assignment of partitions to nodes? 

it is important that all participants agree— otherwise requests would be sent to the wrong nodes and not handled correctly. There are protocols for achieving consensus in a distributed system, but they are hard to implement correctly


![](/img/post/ddia/6-7.png)


- Many distributed data systems <span style="background-color:#FFFF00">**rely on a separate coordination service such as Zoo‐ Keeper to keep track of this cluster metadata**</span>
- Each node registers itself in ZooKeeper, and ZooKeeper maintains the authoritative mapping of partitions to nodes. <span style="background-color:#FFFF00">**每个node 都register iteslf zookeeper maintain data mapping**</span>
- Whenever a partition changes ownership, or a node is added or removed, ZooKeeper notifies the routing tier so that it can keep its routing information up to date. 当partition, <span style="background-color:#FFFF00">**nodes发生改变 会notify routing tier**</span><span style="color:red"></span>
  - For example, LinkedIn’s Espresso uses Helix for cluster management (which in turn relies on ZooKeeper), implementing a routing tier as shown in Figure 6-8. HBase, SolrCloud, and Kafka also use ZooKeeper to track partition assignment. MongoDB has a similar architecture, but it relies on its own config server implementation and mongos daemons as the routing tier

![](/img/post/ddia/6-8.png)


Cassandra and Riak take a different approach

- <span style="background-color:#FFFF00">**gossip protocol**</span> among the
nodes to **disseminate any changes in cluster state**. <span style="background-color:#FFFF00">**Requests can be sent to any node request可以发到任何的node**</span>, and that node forwards them to the appropriate node for the requested partition(approach 1 in Figure 6-7). 
- This model <span style="background-color:#FFFF00">**puts more complexity in the database**</span> nodes but avoids the dependency on an external coordination service such as ZooKeeper
- **Couchbase** does not rebalance automatically, which simplifies the design. Normally it is configured with a routing tier called <span style="background-color:#FFFF00">**moxi**</span>, which learns about routing changes from the cluster nodes
- When using a routing tier or when sending requests to a random node, clients still need to <span style="color:red">find the IP addresses to connect to</span>. These are not as fast-changing as the assignment of partitions to nodes, so it is often sufficient to use **DNS** for this purpose

#### Parallel Query Execution

- <span style="background-color:#FFFF00">**massively parallel processing (MPP)**</span> relational database products
- A typical data warehouse query contains several join, filtering, grouping, and aggregation operations.
  - The MPP query optimizer breaks this complex query into a number of execution stages and partitions, many of which can be executed in parallel on different nodes of the database cluster. <span style="background-color:#FFFF00">**MPP把query 变成number of execution stages, executed in parallel.**</span>
  - Queries that involve <span style="background-color:#FFFF00">**scanning over large parts of the dataset particularly benefit from such parallel execution**</span>

## Transactions


A <span style="background-color:#FFFF00">**transaction**</span> is a way for an application to group several reads and writes together into a logical unit. Conceptually, all the reads and writes in a transaction are executed as one operation: either the entire transaction succeeds (commit) or it fails(abort, rollback). If it fails, the application can safely retry. With transactions, error handling becomes much simpler for an application, because it doesn’t need to worry about partial failure. 一个transcation 要么全部成功，要么全部失败

#### The Meaning of ACID

<span style="background-color:#FFFF00">**Atomicity, Consistency, Isolation, and Dura‐bility**</span>

**Atomicity**： 

- atomic refers to something that cannot be broken down into smaller parts. 
- if one thread executes an atomic operation, that <span style="background-color:#FFFF00">**means there is no way that another thread could see the half-finished result of the operatio**</span>n. The system can only be in the state it was before the operation or after the operation, not something in between.
-  <span style="background-color:#FFFF00">**not about concurrency**</span>. not describe what happens if several processes try to access the same data at the same time, because that is covered under the letter I, for isolation 
-  ACID atomicity <span style="background-color:#FFFF00">**describes what happens if a client wants to make several writes, but a fault occurs after some of the writes have been processed**</span>—for example, a process crashes, a network connection is interrupted, a disk becomes full, or some integrity constraint is violated. <span style="background-color:#FFFF00">**If the writes are grouped together into an atomic transaction, and the transaction cannot be completed (committed) due to a fault 如果writes group together into atomic transaction不能完成因为fault**</span>, then the transaction is aborted and the database must discard or undo any writes it has made so far in that transaction.
-  Without atomicity, 如果有error ocurrs partway through making 多个chagnes, 不知道which changes taken effect and which haven't. 如果重试，risk making the same change twice, leading to duplicate or incorrect data. 
   -  <span style="background-color:#FFFF00">**Atomicity simplifies this problem: if a transaction was aborted, the application can be sure that it didn’t change anything, so it can safely be retried**</span>.
-  The ability to abort a transaction on error and have all writes from that transaction discarded is the defining feature of <span style="color:purple">ACID atomicity</span> <span style="background-color:#FFFF00">**如果有一个错误，全部丢弃
**</span>


**Consistency**


• In Chapter 5 we discussed replica consistency and the issue of eventual consistency that arises in asynchronously replicated systems (see “Problems with Replication Lag” on page 161).
• Consistent hashing is an approach to partitioning that some systems use for rebalancing (see “Consistent Hashing” on page 204).
• In the CAP theorem (see Chapter 9), the word consistency is used to mean linearizability

- In the context of ACID, consistency refers to an application-specific notion of the database being in a <span style="background-color:#FFFF00">**“good state“**</span>
- this idea of consistency depends on the application’s notion of invariants（data）, and it’s the<span style="color:red"> application’s responsibility to define its transactions correctly so that they preserve consistency</span>. This is not something that the database can guarantee: if you write bad data that violates your invariants, the database can’t stop you (some 可以被检查，比如foreign key constraint or uniqueness constraint)
- <span style="background-color:#FFFF00">**Atomicity, isolation, and durability are properties of the database, whereas consistency (in the ACID sense) is a property of the application**</span>


**Isolation**

- accessing the same database records, 可能有concurrency problems
- Isolation in the sense of ACID means that <span style="background-color:#FFFF00">**concurrently executing transactions are isolated from each other 同时的statement是相互独立的**</span>。 cannot step on each other’s toes
  - The classic database textbooks formalize isolation as <span style="color:red">**serializability**</span>, which means that each transaction can pretend that it is the only transaction running on the entire database (<span style="background-color:#FFFF00">每个transaction 都假设是only transaction running on entire database</span>)
  - The database ensures that when the transactions have committed, the result is the same as if they had run serially (one after another), even though in reality they may have run concurrently <span style="background-color:#FFFF00">**数据库确保transaction committed, result是一样的就像run serially**</span>
- in practice, serializable isolation is rarely used, because it carries a performance penalty. <span style="background-color:#FFFF00">**实际上serializable isolation很少被用，因为performance原因**</span>
- Oracle implement called <span style="background-color:#FFFF00">**snapshot isolatio**</span>n, eaker guarantee than serializability.

**Durability**

- Durability is the promise that once a transaction has committed successfully, any data it has written will not be forgotten, even if there is a hardware fault or the database crashes (<span style="background-color:#FFFF00">**Durability 保证当transaction commited, 任何data都不会被丢失, 即使database crash**</span>)
- In a single-node database, durability typically means that the data has been written to nonvolatile storage such as a <span style="color:purple">**hard drive or SSD**</span>
  - 通常invole write-ahead log or similar, <span style="color:red">which allows recovery in the event that the data structures on disk are corrupted</span>.
- In a replicated database, **durability may mean that the data has been successfully copied to some number of nodes 表示data 成功被复制到一些node**. 为了提供durability guarantee, <span style="background-color:#FFFF00">**a database必须wait until these writes or replications are complete**</span> before a transaction as succesfully committed.
- <span style="background-color:#FFFF00">**perfect durability does not exist**</span>: if all your hard disks and all your backups are destroyed at the same time, there’s obviously nothing your database can do to save you


<span style="background-color:#FFFF00">**The truth is, nothing is perfect**</span>. In practice, there is no one technique that can provide absolute guarantees. various risk-reduction techniques, including writing to disk, replicating to remote machines, and backups—and they can and should be used together. <span style="background-color:#FFFF00">** 不可能提供absolute的保证，只能尽可能提供risk-reduction techniques 包括writing to disk, 复制到remote machine 和back-up**</span>

比如

- if write to disk and machine dies, 数据没有丢失 但是inaccessible, either fix machine or transfer the disk to another machine
- A correlated fault: a power outage or a bug that crashes every node on partiular input - can knock out all replicas at once
- asynchronously replicated system, recent writes may be lost when the leader becomes unavailable
- When the power is suddenly cut, SSD show sometimes violate the guarantee that they provide: even `fsync` (write all modified data for a given file to the disk) isn't guaranteed to work correctly.
- subtle interactions between storage enigne and filesystem implementation can lead to bugs that are hard to track down.
- Data on disk become corrupted wihtout being detected.  如果data corrupted for some time, replicas and recent backups 可能也corrupted
- One study of SSDs found that between 30% and 80% of drives develop at least one bad block during the first four years of operation. <span style="color:red">Magnetic hard drives have a lower rate of bad sectors, but a higher rate of complete failure than SSD</span>
- If an SSD is disconnected from power, it can start losing data within a few weeks, depending on the temperature



#### Single-Object and Multi-Object Operations

**Atomicity**

If an error occurs halfway through a sequence of writes, the transaction should be aborted, and the writes made up to that point should be discarded. In other words, <span style="background-color:#FFFF00">**the database saves you from having to worry about partial failure**</span>, by giving an all-or-nothing guarantee.

**Isolation**

<span style="background-color:#FFFF00">**Concurrently running transactions shouldn’t interfere with each other**</span>. For example, if one transaction makes several writes, then another transaction should see either all or none of those writes, but not some subset.

下图 illustrates the need for atomicity: if an error occurs somewhere over the course of the transaction, the contents of the mailbox and the unread counter might become out of sync. In an atomic transaction, <span style="background-color:#FFFF00">**if the update to the counter fails, the transaction is aborted and the inserted email is rolled back**</span>.

![](/img/post/ddia/7-3.png)


Multi-object transactions <span style="background-color:#FFFF00">**require some way of determining which read and write operations belong to the same transaction**</span>. In relational database, typically done based on the client’s TCP connection to the database server. on any particular connection, everything between a `BEGIN TRANSACTION` and a `COMMIT` statement is considered to be part of the same transaction  (This is not ideal. If the TCP connection is interrupted, the transaction must be aborted. If the interruption happens after the client has requested a commit but before the server acknowledges that the commit happened, the client doesn’t know whether the transaction was committed or not. To solve this issue, a transaction manager can group operations by a unique transaction identifier that is not bound to a particular TCP connection)


对于nonrelational database, don't have such a way of grouping operations together. Even if there is a multi-object API. that doesn’t necessarily mean it has transaction semantics. Command 可能成功for some keys and fail for others, leaving database in partially updated state. 


**Single-object writes**

storage engines almost universally aim to provide atomicity and isolation on the level of a single object on one node.


**Handling errors and aborts**

A key feature of a transaction is that it can be aborted and safely retried if an error occurred. If the database is in danger of violating its guarantee of atomicity, isolation, or durability, <span style="background-color:#FFFF00">**it would rather abandon the transaction entirely than allow it to remain half-finished**</span>.

Not all systems follow that philosophy, 比如<span style="background-color:#FFFF00">**leaderless replication**</span> - will do as much as it can, and if it runs into error, won't undo something it has already done. - <span style="background-color:#FFFF00">**it's application's responsibility to recover from errors**</span>

retrying an aborted transaction is a simple and effective error handling 但不是perfect 

- 如果transaction succeed, but network failed while server tried to ack successful commit to client, retrying the transaction cuase it to be performed twice - unless you have application level dedup mechansim
- If error due to overload, retrying transaction make problem worse. To avoid such feedback cycles, limit the number of retries, use exponential backoffs, and handle overload-related errors differently from other errors
- <span style="background-color:#FFFF00">**only worth retrying after transient errors (比如deadlock, isolation violation, temporary network interruptions, and failover). After a permanent error, a retry pointless**</span>
- If the transaction also has side effects outside of the database, those side effects may happen even if the transaction is aborted. 比如发送邮件，不想send email again everyt time you retry the transaction
- If client process fails while retrying, any data it was trying to write to the database is lost

#### Weak Isolation Levels

If two transactions don’t touch the same data, they can safely be run in parallel, because neither depends on the other. 

因为concurrency issue 很难reproduce, <span style="background-color:#FFFF00">**databases have long tried to hide concurrency issues from application developers by providing transaction isolation**</span>.

In practice, isolation is unfortunately not that simple. <span style="color:red">**Serializable isolation has a performance cost**</span>, and many databases don’t want to pay that price. It’s therefore common for systems to use <span style="background-color:#FFFF00">**weaker levels of isolation**</span>, which protect against some concurrency issues, but not all.

#### Read Committed

makes two guarantees

- When reading from the database, you will only see data that has been committed(<span style="background-color:#FFFF00">**no dirty reads**</span>).
- When writing to the database, you will only overwrite data that has been committed (<span style="background-color:#FFFF00">**no dirty writes**</span>).

**No dirty reads**

Imagine a transaction has written some data to the database, but the transaction has not yet committed or aborted. Can another transaction see that uncommitted data? If yes, that is called a <span style="color:purple">**dirty read**</span>

<span style="background-color:#FFFF00">**Transaction running at read committed isolation level must prevent dirty reads**</span>. Any writes by a transaction only become visible to others when that transaction commits.

![](/img/post/ddia/7-4.png)

比如上图 where user 1 has set x = 3, but user 2’s get x still returns the old value, 2, while user 1 has not yet committed


- If a transaction needs to update several objects, a dirty read means that another transaction may see some of the updates but not others. <span style="color:red">Seeing the database in a partially updated state is confusing to users and may cause other transactions to take incorrect decisions</span>.
- If a transaction aborts, any writes it has made need to be rolled back. <span style="background-color:#FFFF00">**If the database allows dirty reads, that means a transaction may see data that is later rolled back**</span>—i.e., which is never actually committed to the database. Reasoning about the consequences quickly becomes mind-bending.
