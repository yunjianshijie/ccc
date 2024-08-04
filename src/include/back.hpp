#define HAND 8087
#define ServerAddr "0.0.0.0"
#define MAX_EVENTS 40

#define EXIT -11
#define SUCCESS 0
#define FAILURE -1
// 测试

// 登录
#define LOGIN 1
// 注册
#define REGISTER 2
// 退出
#define EXIT_SUCCESS1 3
// 忘记密码
#define FORGET_PASSWORD 4
// 登录成功
#define LOGIN_SUCCESS 5
// 注册失败_用户名已存在
#define CHANGE_PASSWORD 6
// 改变密码
#define REGISTER_FAILURE_PWD 7
// 登录失败_用户名或密码错误
#define LOGIN_FAILURE 8
// 登录成功 返回主页

// 主页
#define HOME 9

// 好友
// 修改昵称
#define CHANGE_NAME 10
// 添加好友
#define FRIEND_ADD 11
// 删除好友
#define FRIEND_DELETE 12
// 好友消息
#define FRIEND_ADD_2 13
// 发送好友消息
#define FRIEND_MESSAGE_SEND 14
// 接收好友消息
#define FRIEND_MESSAGE_RECEIVE 15
// 删除好友消息
#define FRIEND_MESSAGE_DELETE 16
// 注销账号1
#define DELETE_SELF1 17
// 注销账号2
#define DELETE_SELF2 18

// 用户查询
#define USER_QUERY 19
// 好友申请列表
#define FRIEND_APPLY_LIST 20
// 处理好友申请
#define FRIEND_APPLY 21
// 查看好友列表
#define FRIEND_LIST 22

// 私聊
//  聊天消息
#define FRIEND_MESSAGE 23
// 离线消息
#define OFFLINE_MESSAGE 24
// 历史消息
#define HISTORY_MESSAGE 25
// 好友屏蔽
#define FRIEND_SHIELD 26
// 好友取消屏蔽
#define FRIEND_UNSHIELD 27
// 被屏蔽了
#define FRIEND_SHIELDED 28

// 被删好友了，嘤嘤嘤
#define FRIEND_DLEED 29
//
#define MAX 100

//
// 群组
// 创建群聊
#define GROUP_CREATE 30
// 群聊邀请成员
// 群聊消息
#define GROUP_MESSAGE 31

#define GROUP_INVITE 32
// 查看群聊列表
#define GROUP_LIST 33
// 进入群聊
#define GROUP_ENTER 34
// 群聊
#define GROUP_CHAT 35
//
#define GROUP_ADD 36
//
#define GROUP_ADD2 37
//
#define GROUP_MANAGE 38

// 修改密码
#define CHANGE_PASS 39
#define CHANGE_PASS1 40

// 群主!
#define OWNER 41
// 管理员*
#define MANAGER 42
// 成员&
#define MEMBER 43
//%
// 踢出群聊(群主)
#define GROUP_KICK_O 44
// 提出群聊(管理员)
#define GROUP_KICK_M 45

// 邀请群员

// 查看群成员
#define GROUP_LOOK_NUM 46

//
#define GROUP_LEAVE 47

#define GROUP_LEAVE_O 48
// 设置管理
#define GROUP_SET_MA 49
// 设置禁言
#define GROUP_SET_BAN_O 50
// 管理员踢出
#define GROUP_KICK 51
// 管理员设置禁言
#define GROUP_SET_BAN 52
// 查看申请列表
#define GROUP_WILL_LIST 53
// 处理申请
#define GROUP_CHULI_WILL 54
// 群聊历史消息
#define GROUP_HIS 55
// 文件上传
#define FILE_SEND 56
// 文件接收
#define FILE_RECEIVE_G 57
//
#define FILE_RECEIVE_G1 58
// 好友文件上传
#define FILE_SEND_F 59
//
#define FILE_RECEIVE_F 60

//
#define FILE_RECEIVE 61

#define TIMEOUT_SECONDS 5
#define GROUP_INVITE_Q 62

#define FRIEND_APPLY_Q 63