#include "EventDispatch.h"
#include "BaseSocket.h"

#define MIN_TIMER_DURATION	100	// 100 miliseconds

CEventDispatch* CEventDispatch::m_pEventDispatch = NULL;


void CEventDispatch::AddTimer(callback_t callback, void* user_data, uint64_t interval)
{
	list<TimerItem*>::iterator it;
	for (it = m_timer_list.begin(); it != m_timer_list.end(); it++)
	{
		TimerItem* pItem = *it;
		if (pItem->callback == callback && pItem->user_data == user_data)
		{
			pItem->interval = interval;
			pItem->next_tick = get_tick_count() + interval;
			return;
		}
	}

	TimerItem* pItem = new TimerItem;
	pItem->callback = callback;
	pItem->user_data = user_data;
	pItem->interval = interval;
	pItem->next_tick = get_tick_count() + interval;
	m_timer_list.push_back(pItem);
}

void CEventDispatch::RemoveTimer(callback_t callback, void* user_data)
{
	list<TimerItem*>::iterator it;
	for (it = m_timer_list.begin(); it != m_timer_list.end(); it++)
	{
		TimerItem* pItem = *it;
		if (pItem->callback == callback && pItem->user_data == user_data)
		{
			m_timer_list.erase(it);
			delete pItem;
			return;
		}
	}
}

void CEventDispatch::_CheckTimer()
{
	uint64_t curr_tick = get_tick_count();
	list<TimerItem*>::iterator it;

	for (it = m_timer_list.begin(); it != m_timer_list.end(); )
	{
		TimerItem* pItem = *it;
		it++;		// iterator maybe deleted in the callback, so we should increment it before callback
		if (curr_tick >= pItem->next_tick)
		{
			pItem->next_tick += pItem->interval;
			pItem->callback(pItem->user_data, NETLIB_MSG_TIMER, 0, NULL);
		}
	}
}


CEventDispatch::CEventDispatch()
{
	m_epfd = epoll_create(1024);
	if (m_epfd == -1)
	{
		log("epoll_create failed\n");
	}
}

CEventDispatch::~CEventDispatch()
{
	close(m_epfd);
}


CEventDispatch* CEventDispatch::Instance()
{
	if (m_pEventDispatch == NULL)
	{
		m_pEventDispatch = new CEventDispatch();
	}

	return m_pEventDispatch;
}


void CEventDispatch::AddEvent(SOCKET fd, uint8_t socket_event)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLPRI | EPOLLERR | EPOLLHUP;
	ev.data.fd = fd;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) != 0)
	{
		log("epoll_ctl() failed, errno=\n", errno);
	}
}

void CEventDispatch::RemoveEvent(SOCKET fd, uint8_t socket_event)
{
	if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL) != 0)
	{
		log("epoll_ctl failed, errno=%d\n", errno);
	}
}

void CEventDispatch::StartDispatch()
{
	struct epoll_event events[1024];
	int nfds = 0;

	while (true)
	{
		nfds = epoll_wait(m_epfd, events, 1024, MIN_TIMER_DURATION);
		for (int i = 0; i < nfds; i++)
		{
			int ev_fd = events[i].data.fd;
			CBaseSocket* pSocket = FindBaseSocket(ev_fd);
			if (!pSocket)
				continue;

			if (events[i].events & EPOLLIN)
			{
				log("OnRead, socket=%d\n", ev_fd);
				pSocket->OnRead();
			}

			if (events[i].events & EPOLLOUT)
			{
				log("OnWrite, socket=%d\n", ev_fd);
				pSocket->OnWrite();
			}

			if (events[i].events & (EPOLLPRI | EPOLLERR | EPOLLHUP))
			{
				log("OnClose, socket=%d\n", ev_fd);
				pSocket->OnClose();
			}

			pSocket->ReleaseRef();
		}
		_CheckTimer();

	}
}
