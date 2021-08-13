# redismodule_qxiu ()
你是否一直觉得，redis的zset 和 hset缺个东西？使的不少场景下程序变得复杂？ 
没错，它缺个value. 本插件新增 zmap和hnmap和looparr三种数据结构  

zmap：是在zset基础上多存个value。 
经典场景：一篇文章的评论，评论按点赞排序。key是文章ID，field是评论ID，点赞数是score。好，zset没地方存评论内容了。你需要单独找地方去存。而我的zmap解决了它。  

hnset：当你把hset的value当一个数字incr时，你是否又发现缺了个value?hnset就是解决这个场景的。  

looparr：乃是定长的容器，适合存储最近的历史数据。
