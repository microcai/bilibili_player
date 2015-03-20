
# bilibili 弹幕视频播放器

Linux 版 bilibili 因为 flash 版本停滞于 11 的原因, 许多视频无限小电视而无法观看.
虽然用 F12 大法可以获取视频地址然后 vlc 播放, 但是木弹幕了啊! 没弹幕我还不如直接下高清的看了.
无吐槽不补番!

于是就写了一个这个简单的播放器


用法: 直接跟B站播放页面的地址

	bilibili_player http://www.bilibili.com/video/av2037598/index_3.html


# 编译 bilibili_player

## 依赖

bilibili_player 依赖 Qt5Widgets Qt5Multimedia Qt5Network Qt5Xml 以及开源物理引擎 [bullet](https://github.com/bulletphysics/bullet3)

## 编译

Gentoo 用户请使用 [gentoo-zh overlay](https://github.com/microcai/gentoo-zh)

	layman -f -a gentoo-zh
	emerge bilibili_player

其他用户， 请使用 cmake 编译。 需要 cmake >= 3.1



