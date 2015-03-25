
# bilibili 弹幕视频播放器

Linux 版 bilibili 因为 flash 版本停滞于 11 的原因, 许多视频无限小电视而无法观看.
虽然用 F12 大法可以获取视频地址然后 vlc 播放, 但是木弹幕了啊! 没弹幕我还不如直接下高清的看了.
无吐槽不补番!

于是就写了一个这个简单的播放器。

PS： 如果使用 4k 屏幕，你还是使用本播放器吧。。。flash根本就无视系统DPI设置。。。

用法: 直接跟B站播放页面的地址

	bilibili_player http://www.bilibili.com/video/av2037598/index_3.html


# 编译 bilibili_player

## 依赖

bilibili_player 依赖 Qt5Widgets Qt5Multimedia Qt5Network Qt5Xml 以及开源物理引擎 [bullet](https://github.com/bulletphysics/bullet3)

### OpenGL 注意事项

bilibili_player 使用 OpenGL 硬件加速绘制视频和弹幕。确保您的显卡支持 OpenGL >= 3.0。通常来说近十年内生产的显卡都支持。
如果opengl下发现黑屏白屏红屏之类的情况，请试试看加 --nogl 参数。

## 编译

Gentoo 用户请使用 [gentoo-zh overlay](https://github.com/microcai/gentoo-zh)

	layman -f -a gentoo-zh
	emerge bilibili-player

其他用户， 请使用 cmake 编译。 需要 cmake >= 3.1 (是的，就歧视ubnutu 用户怎么着？ 谁用谁傻逼。 老版本=稳定？ 稳定你麻痹)

## 播放控制

软件界面很简洁，颇有 mplayer 的风格。控制都使用键盘进行。除了那个进度条君。
按下空格键暂停和恢复播放，按下 Ctrl + 方向盘右键 快进 30s. 按下 F 键切换全屏。
按下 Ctrl+{1,2,3,4,5,6} 分别进入 100% 200% 300% 400% 500% 600% 放大状态。


