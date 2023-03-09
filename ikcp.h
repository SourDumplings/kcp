//=====================================================================
//
// KCP - A Better ARQ Protocol Implementation
// skywind3000 (at) gmail.com, 2010-2011
//
// Features:
// + Average RTT reduce 30% - 40% vs traditional ARQ like tcp.
// + Maximum RTT reduce three times vs tcp.
// + Lightweight, distributed as a single source file.
//
//=====================================================================
#ifndef __IKCP_H__
#define __IKCP_H__

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

//=====================================================================
// 32BIT INTEGER DEFINITION
//=====================================================================
#ifndef __INTEGER_32_BITS__
#define __INTEGER_32_BITS__
#if defined(_WIN64) || defined(WIN64) || defined(__amd64__) ||      \
	defined(__x86_64) || defined(__x86_64__) || defined(_M_IA64) || \
	defined(_M_AMD64)
typedef unsigned int ISTDUINT32;
typedef int ISTDINT32;
#elif defined(_WIN32) || defined(WIN32) || defined(__i386__) || \
	defined(__i386) || defined(_M_X86)
typedef unsigned long ISTDUINT32;
typedef long ISTDINT32;
#elif defined(__MACOS__)
typedef UInt32 ISTDUINT32;
typedef SInt32 ISTDINT32;
#elif defined(__APPLE__) && defined(__MACH__)
#include <sys/types.h>
typedef u_int32_t ISTDUINT32;
typedef int32_t ISTDINT32;
#elif defined(__BEOS__)
#include <sys/inttypes.h>
typedef u_int32_t ISTDUINT32;
typedef int32_t ISTDINT32;
#elif (defined(_MSC_VER) || defined(__BORLANDC__)) && (!defined(__MSDOS__))
typedef unsigned __int32 ISTDUINT32;
typedef __int32 ISTDINT32;
#elif defined(__GNUC__)
#include <stdint.h>
typedef uint32_t ISTDUINT32;
typedef int32_t ISTDINT32;
#else
typedef unsigned long ISTDUINT32;
typedef long ISTDINT32;
#endif
#endif

//=====================================================================
// Integer Definition
//=====================================================================
#ifndef __IINT8_DEFINED
#define __IINT8_DEFINED
typedef char IINT8;
#endif

#ifndef __IUINT8_DEFINED
#define __IUINT8_DEFINED
typedef unsigned char IUINT8;
#endif

#ifndef __IUINT16_DEFINED
#define __IUINT16_DEFINED
typedef unsigned short IUINT16;
#endif

#ifndef __IINT16_DEFINED
#define __IINT16_DEFINED
typedef short IINT16;
#endif

#ifndef __IINT32_DEFINED
#define __IINT32_DEFINED
typedef ISTDINT32 IINT32;
#endif

#ifndef __IUINT32_DEFINED
#define __IUINT32_DEFINED
typedef ISTDUINT32 IUINT32;
#endif

#ifndef __IINT64_DEFINED
#define __IINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 IINT64;
#else
typedef long long IINT64;
#endif
#endif

#ifndef __IUINT64_DEFINED
#define __IUINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 IUINT64;
#else
typedef unsigned long long IUINT64;
#endif
#endif

#ifndef INLINE
#if defined(__GNUC__)

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE __inline__ __attribute__((always_inline))
#else
#define INLINE __inline__
#endif

#elif (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE
#endif
#endif

#if (!defined(__cplusplus)) && (!defined(inline))
#define inline INLINE
#endif

//=====================================================================
// QUEUE DEFINITION
//=====================================================================
#ifndef __IQUEUE_DEF__
#define __IQUEUE_DEF__

struct IQUEUEHEAD
{
	struct IQUEUEHEAD *next, *prev;
};

typedef struct IQUEUEHEAD iqueue_head;

//---------------------------------------------------------------------
// queue init
//---------------------------------------------------------------------
#define IQUEUE_HEAD_INIT(name) \
	{                          \
		&(name), &(name)       \
	}
#define IQUEUE_HEAD(name) \
	struct IQUEUEHEAD name = IQUEUE_HEAD_INIT(name)

#define IQUEUE_INIT(ptr) ( \
	(ptr)->next = (ptr), (ptr)->prev = (ptr))

#define IOFFSETOF(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)

#define ICONTAINEROF(ptr, type, member) ( \
	(type *)(((char *)((type *)ptr)) - IOFFSETOF(type, member)))

#define IQUEUE_ENTRY(ptr, type, member) ICONTAINEROF(ptr, type, member)

//---------------------------------------------------------------------
// queue operation
//---------------------------------------------------------------------
#define IQUEUE_ADD(node, head) (                        \
	(node)->prev = (head), (node)->next = (head)->next, \
	(head)->next->prev = (node), (head)->next = (node))

#define IQUEUE_ADD_TAIL(node, head) (                   \
	(node)->prev = (head)->prev, (node)->next = (head), \
	(head)->prev->next = (node), (head)->prev = (node))

#define IQUEUE_DEL_BETWEEN(p, n) ((n)->prev = (p), (p)->next = (n))

#define IQUEUE_DEL(entry) (              \
	(entry)->next->prev = (entry)->prev, \
	(entry)->prev->next = (entry)->next, \
	(entry)->next = 0, (entry)->prev = 0)

#define IQUEUE_DEL_INIT(entry) \
	do                         \
	{                          \
		IQUEUE_DEL(entry);     \
		IQUEUE_INIT(entry);    \
	} while (0)

#define IQUEUE_IS_EMPTY(entry) ((entry) == (entry)->next)

#define iqueue_init IQUEUE_INIT
#define iqueue_entry IQUEUE_ENTRY
#define iqueue_add IQUEUE_ADD
#define iqueue_add_tail IQUEUE_ADD_TAIL
#define iqueue_del IQUEUE_DEL
#define iqueue_del_init IQUEUE_DEL_INIT
#define iqueue_is_empty IQUEUE_IS_EMPTY

#define IQUEUE_FOREACH(iterator, head, TYPE, MEMBER)            \
	for ((iterator) = iqueue_entry((head)->next, TYPE, MEMBER); \
		 &((iterator)->MEMBER) != (head);                       \
		 (iterator) = iqueue_entry((iterator)->MEMBER.next, TYPE, MEMBER))

#define iqueue_foreach(iterator, head, TYPE, MEMBER) \
	IQUEUE_FOREACH(iterator, head, TYPE, MEMBER)

#define iqueue_foreach_entry(pos, head) \
	for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)

#define __iqueue_splice(list, head)                              \
	do                                                           \
	{                                                            \
		iqueue_head *first = (list)->next, *last = (list)->prev; \
		iqueue_head *at = (head)->next;                          \
		(first)->prev = (head), (head)->next = (first);          \
		(last)->next = (at), (at)->prev = (last);                \
	} while (0)

#define iqueue_splice(list, head)        \
	do                                   \
	{                                    \
		if (!iqueue_is_empty(list))      \
			__iqueue_splice(list, head); \
	} while (0)

#define iqueue_splice_init(list, head) \
	do                                 \
	{                                  \
		iqueue_splice(list, head);     \
		iqueue_init(list);             \
	} while (0)

#ifdef _MSC_VER
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
#pragma warning(disable : 4996)
#endif

#endif

//---------------------------------------------------------------------
// BYTE ORDER & ALIGNMENT
//---------------------------------------------------------------------
#ifndef IWORDS_BIG_ENDIAN
#ifdef _BIG_ENDIAN_
#if _BIG_ENDIAN_
#define IWORDS_BIG_ENDIAN 1
#endif
#endif
#ifndef IWORDS_BIG_ENDIAN
#if defined(__hppa__) ||                                           \
	defined(__m68k__) || defined(mc68000) || defined(_M_M68K) ||   \
	(defined(__MIPS__) && defined(__MIPSEB__)) ||                  \
	defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
	defined(__sparc__) || defined(__powerpc__) ||                  \
	defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
#define IWORDS_BIG_ENDIAN 1
#endif
#endif
#ifndef IWORDS_BIG_ENDIAN
#define IWORDS_BIG_ENDIAN 0
#endif
#endif

#ifndef IWORDS_MUST_ALIGN
#if defined(__i386__) || defined(__i386) || defined(_i386_)
#define IWORDS_MUST_ALIGN 0
#elif defined(_M_IX86) || defined(_X86_) || defined(__x86_64__)
#define IWORDS_MUST_ALIGN 0
#elif defined(__amd64) || defined(__amd64__)
#define IWORDS_MUST_ALIGN 0
#else
#define IWORDS_MUST_ALIGN 1
#endif
#endif

//=====================================================================
// SEGMENT 即 KCP 报文段
//=====================================================================
struct IKCPSEG
{
	// 链表节点，发送和接受队列都是这里的链表的结构
	struct IQUEUEHEAD node;

	// 会话编号，同一个会话编号相同
	IUINT32 conv;

	// 报文段类型，譬如 DATA 或者 ACK，一共有四种
	IUINT32 cmd;

	// 由于 MTU 的限制，大报文段会拆分成多个小报文段，这个是小报文段的编号
	IUINT32 frg;

	// 每个报文段，都会附带上发送方的接受窗口大小
	IUINT32 wnd;

	// 发送时间，如果是 ACK 包，会设置为源报文段的 ts
	IUINT32 ts;

	// 唯一标识报文段的编号
	// KCP 给每个报文段都分配一个 sn 号，接收端收到报文段后回给发送端的 ACK 报文段的 sn 号与之相同
	IUINT32 sn;

	// 代表小于 una 的报文段都接收成功，跟 TCP 含义一致：oldest unacknowledged sequence number SND
	// 同时这个字段也表示期待下一个报文段的 sn 号
	IUINT32 una;

	// 数据长度
	IUINT32 len;

	// 重传时间戳. 超过这个时间表示该报文超时, 需要重传
	IUINT32 resendts;

	// RTO, 下次超时等待时间
	IUINT32 rto;

	// ACK 失序次数. 也就是 KCP Readme 中所说的 "跳过" 次数.
	IUINT32 fastack;

	// 发送次数
	IUINT32 xmit;

	// 数据，只有数据报文会有这个字段
	char data[1];
};

//---------------------------------------------------------------------
// IKCPCB KCP 结构
//---------------------------------------------------------------------
struct IKCPCB
{
	// conv: 会话编号
	// mtu, mss: 最大传输单元，最大报文段大小，mss = mtu - 包头长度(24)
	// state: 会话状态，0 有效，-1 断开(注意 state 是 unsigned int, -1 实际上是 0xffffffff)
	IUINT32 conv, mtu, mss, state;

	// snd_una: 发送缓冲区中最小还未确认送达的报文段的编号. 也就是说, 编号比它小的报文段都已确认送达.
	// snd_nxt: 下一个等待发送的报文段编号
	// rcv_nxt: 下一个等待接收的报文段编号
	IUINT32 snd_una, snd_nxt, rcv_nxt;

	// ts_recent, ts_lastack: 未用到
	// ssthresh: 拥塞控制慢启动阈值
	IUINT32 ts_recent, ts_lastack, ssthresh;

	// rx_rto: rto (retransmission timeout)，超时重传时间
	// rx_rttval, rx_srtt, rx_minrto: 计算 rto 的中间变量
	IINT32 rx_rttval, rx_srtt, rx_rto, rx_minrto;

	// snd_wnd, rcv_wnd: 最大发送和接收窗口大小
	// rmt_wnd: remote wnd ，对端剩余接受窗口大小
	// cwnd: congestion window, 拥塞窗口. 用于拥塞控制.
	// probe: 是否要发送控制报文的标志
	IUINT32 snd_wnd, rcv_wnd, rmt_wnd, cwnd, probe;

	// current: 当前时间
	// interval: flush 的时间粒度
	// ts_flush: 下次需要 flush 的时间
	// xmit: 发送失败次数，即该链接超时重传的总次数.
	IUINT32 current, interval, ts_flush, xmit;

	// 对应链表的长度
	// 接收缓冲区, 发送缓冲区, 接收队列, 发送队列的长度
	IUINT32 nrcv_buf, nsnd_buf;
	IUINT32 nrcv_que, nsnd_que;

	// nodelay: 是否启动快速模式. 用于控制 RTO 增长速度
	// updated: 是否调用过 ikcp_update
	IUINT32 nodelay, updated;

	// ts_probe, probe_wait: 确定何时需要发送窗口询问报文, 对端接收窗口长时间为 0 时主动定期发起询问
	IUINT32 ts_probe, probe_wait;

	// deal_link: 对端长时间无应答, 当一个报文发送超时次数达到 `dead_link` 次时认为连接断开
	// incr: 参与计算发送窗口 cwnd 大小
	IUINT32 dead_link, incr;

	// queue: 跟用户层接触的报文段
	// buf: 协议缓存的报文段
	struct IQUEUEHEAD snd_queue; // 发送队列
	struct IQUEUEHEAD rcv_queue; // 接受队列
	struct IQUEUEHEAD snd_buf; // 发送缓冲区
	struct IQUEUEHEAD rcv_buf; // 接收缓冲区

	// 需要发送 ack 的报文段信息
	IUINT32 *acklist;

	// 需要 ack 的包数量
	IUINT32 ackcount;

	// acklist 内存大小
	IUINT32 ackblock;

	// 用户层传进来的数据
	void *user;

	// 存放一个 kcp 包的空间, flush 时用到的临时缓冲区
	char *buffer;

	// ACK 失序 fastresend 次时触发快速重传
	int fastresend;

	// 快速重传最大次数, 传输次数小于 fastlimit 的报文才会执行快速重传
	int fastlimit;

	// nocwnd: 是否不考虑拥塞窗口
	// stream: 是否开启流模式, 开启后可能会合并包
	int nocwnd, stream;

	// 用于控制日志
	int logmask;

	// 发送数据接口
	int (*output)(const char *buf, int len, struct IKCPCB *kcp, void *user);

	// 写日志接口
	void (*writelog)(const char *log, struct IKCPCB *kcp, void *user);
};

// 一个 ikcpcb 实例代表一个 KCP 连接
typedef struct IKCPCB ikcpcb;

#define IKCP_LOG_OUTPUT 1
#define IKCP_LOG_INPUT 2
#define IKCP_LOG_SEND 4
#define IKCP_LOG_RECV 8
#define IKCP_LOG_IN_DATA 16
#define IKCP_LOG_IN_ACK 32
#define IKCP_LOG_IN_PROBE 64
#define IKCP_LOG_IN_WINS 128
#define IKCP_LOG_OUT_DATA 256
#define IKCP_LOG_OUT_ACK 512
#define IKCP_LOG_OUT_PROBE 1024
#define IKCP_LOG_OUT_WINS 2048

#ifdef __cplusplus
extern "C"
{
#endif

	//---------------------------------------------------------------------
	// interface
	//---------------------------------------------------------------------

	// 创建一个 KCP 实例
	// create a new kcp control object, 'conv' must equal in two endpoint
	// from the same connection. 'user' will be passed to the output callback
	// output callback can be setup like this: 'kcp->output = my_udp_output'
	ikcpcb *ikcp_create(IUINT32 conv, void *user);

	// 释放一个 KCP 实例
	// release kcp control object
	void ikcp_release(ikcpcb *kcp);

	// 设置下层协议输出回调函数
	// set output callback, which will be invoked by kcp
	void ikcp_setoutput(ikcpcb *kcp, int (*output)(const char *buf, int len,
												   ikcpcb *kcp, void *user));

	// 上层可以使用 ikcp_recv 从 KCP 获取数据
	// user/upper level recv: returns size, returns below zero for EAGAIN
	int ikcp_recv(ikcpcb *kcp, char *buffer, int len);

	// 发送数据接口，用户调用 ikcp_send 来让 kcp 发送数据
	// user/upper level send, returns below zero for error
	int ikcp_send(ikcpcb *kcp, const char *buffer, int len);

	// 时钟更新
	// update state (call it repeatedly, every 10ms-100ms), or you can ask
	// ikcp_check when to call it again (without ikcp_input/_send calling).
	// 'current' - current timestamp in millisec.
	void ikcp_update(ikcpcb *kcp, IUINT32 current);

	// Determine when should you invoke ikcp_update:
	// returns when you should invoke ikcp_update in millisec, if there
	// is no ikcp_input/_send calling. you can call ikcp_update in that
	// time, instead of call update repeatly.
	// Important to reduce unnacessary ikcp_update invoking. use it to
	// schedule ikcp_update (eg. implementing an epoll-like mechanism,
	// or optimize ikcp_update when handling massive kcp connections)
	IUINT32 ikcp_check(const ikcpcb *kcp, IUINT32 current);

	// 下层协议输入
	// when you received a low level packet (eg. UDP packet), call it
	int ikcp_input(ikcpcb *kcp, const char *data, long size);

	// flush 发送缓冲区, 会在 ikcp_update 中调用
	// flush pending data
	void ikcp_flush(ikcpcb *kcp);

	// check the size of next message in the recv queue
	int ikcp_peeksize(const ikcpcb *kcp);

	// change MTU size, default is 1400
	int ikcp_setmtu(ikcpcb *kcp, int mtu);

	// set maximum window size: sndwnd=32, rcvwnd=32 by default
	int ikcp_wndsize(ikcpcb *kcp, int sndwnd, int rcvwnd);

	// get how many packet is waiting to be sent
	int ikcp_waitsnd(const ikcpcb *kcp);

	// fastest: ikcp_nodelay(kcp, 1, 20, 2, 1)
	// nodelay: 0:disable(default), 1:enable
	// interval: internal update timer interval in millisec, default is 100ms
	// resend: 0:disable fast resend(default), 1:enable fast resend
	// nc: 0:normal congestion control(default), 1:disable congestion control
	int ikcp_nodelay(ikcpcb *kcp, int nodelay, int interval, int resend, int nc);

	void ikcp_log(ikcpcb *kcp, int mask, const char *fmt, ...);

	// setup allocator
	void ikcp_allocator(void *(*new_malloc)(size_t), void (*new_free)(void *));

	// read conv
	IUINT32 ikcp_getconv(const void *ptr);

#ifdef __cplusplus
}
#endif

#endif
