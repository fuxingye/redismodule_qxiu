# redismodule_qxiu ![](https://img.shields.io/badge/version-1.0.0-green)![](https://img.shields.io/badge/author-%E4%BB%98%E5%85%B4%E7%83%A8%20%E6%96%B9%E8%B6%85-green)
(中国 浙江齐聚科技北京分公司 齐齐直播 处电交友 作者：付兴烨 方超)

你是否一直觉得，redis的zset 和 hset缺个东西？使的不少场景下程序变得复杂？ 

没错，它缺个value. 本插件新增 zmap和hnmap和looparr三种数据结构  

zmap：是在zset基础上多存个value。 这是从0新写的skiplist加hashmap组合，性能略差于zset,因为多存了value

   	经典场景：一篇文章的评论，评论按点赞排序。key是文章ID，member是评论ID，点赞数是score。
	
	好，zset没地方存评论内容了。
   
   	你需要单独找地方去存。而我的zmap解决了它。  

hnset：当你把hset的value当一个数字incr时，你是否又发现缺了个value?hnset就是解决这个场景的。  

looparr：乃是定长的容器，适合存储最近的历史数据。

以下是全部命令：

	zmap:  (与redis原生zset系列命令基本一致)
		xlq.zmadd key score member value
		xlq.zmaddReOri
		xlq.zmincrby
		xlq.zmincrbyReOri
		xlq.zmrangebyscore
		xlq.zmrevrangebyscore
		xlq.zmscore
		xlq.zminfo
		xlq.zmcard
		xlq.zmrem
		xlq.zmremReOri
		xlq.zmremByScore
		xlq.zmremByScoreReOri
		xlq.zmremRangeByScore
		xlq.zmremRangeByScoreReOri
		xlq.zmremRangeByIndex
		xlq.zmremRangeByIndexReOri
	举例：
	一、
	127.0.0.1:6379> xlq.zmadd key1 1 member1 value1
	(integer) 1
	127.0.0.1:6379> xlq.zmadd key1 2 member2 value2
	(integer) 1
	127.0.0.1:6379> xlq.zmrevrangebyscore key1 -100 100 withscores withvalues limit 0 100
	1) "member2"
	2) (integer) 2
	3) "value2"
	4) "member1"
	5) (integer) 1
	6) "value1"


	hmap: (与redis原生hset系列命令基本一致)
		xlq.hnset
		xlq.hnsetReOri
		xlq.hnincrby
		xlq.hnincrbyReOri
		xlq.hnget
		xlq.hngetall
		xlq.hndel
		xlq.hndelReOri
		xlq.hnlen
		xlq.hnkeys
		xlq.hnexists

	loopstr：
		xlq.loopstrCreate
		xlq.loopstrCreateOrResize
		xlq.loopstrInsert
		xlq.loopstrRevrange
		xlq.loopstrInfo

配套JAVA客户端  https://github.com/fuxingye/qxiu_common_tools  基于jedis3.X
