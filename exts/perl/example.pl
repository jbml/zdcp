#!/usr/bin/perl
use strict;
use Zdcp;
use Data::Dumper;
my $h;

$h = Zdcp::create("config.cfg");
print "handle = $h\n";
my $doc=<<END;
<subject: 诺基亚n81标准配置有哪一些><content: [logo] | 网站首页 | 诺基亚手机大全 | 诺基亚资讯 | 诺基亚手机软件 | 诺基亚手机游戏 | 诺基亚问答 | 手机图片 | 诺基亚手机主题 | 电子书 | 免费铃声 | ● 您现在的位置： 诺基亚手机 &gt; 诺基亚问答 &gt; 诺基亚手机知识问答 &gt; 正文 ● 用户登录 ● 新用户注册 [arrow] 诺基亚n81标准配置有哪一些 诺基亚手 机搜索 诺基亚n81标准配置有哪一些 [ ] [搜索] 相关文章 作者：佚名文章来源：本站原创更新时间：2008-9-29 [发表文章] 诺基亚N81好用吗 诺基亚n81上市时间是 赞助商链接 诺基亚N81标准配置 1、带有遥控装置的 3.5 毫米立体声耳机 (HS-45 + AD-54) 2、CA-101 Micro USB 数据线 3、BP-6MT 电池 4、AC-6C + CA-100C 旅行充电器 5、2GB MicroSD 卡 MU-37 上一篇诺基亚问答：诺基亚n81上市时间是 下一篇诺基亚问答：诺基亚N81好用吗 【发表评论】【加入收藏】【告诉好友】【打印此文】【关闭窗口】::你对此问题的回答:: 昵称： [ ] 评分： ( ) 1分 ( ) 2分 (*) 3分 ( ) 4分 ( ) 5分 [ ] 回答内 [ ]容： [ ] [ ] [ ][ 发表我的评论 ]| 设为首页 | 加入收藏 | 联系站长 | 友情链接 | 版权申明 |版权所有(C) CopyRight 2005-2008, Www.Nokia138.Com, Inc. All Rights Reserved><url: http://www.nokia138.com/quiz/6651.html><category:wenda>
END

my $q;
Zdcp::classify($h, $doc, \$q);
print "class = $q\n";

Zdcp::destroy($h);
