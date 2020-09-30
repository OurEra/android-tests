#include <list>
#include <os_typedefs.h>
#include <os_mutex.h>
#include <os_cond.h>
#include <os_assert.h>
#include <utility_scoped_ptr.h>
#include <utility_circle_queue.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct cq {
  std::list<void *> q;
  uint32_t max_size;
  utility::scoped_ptr<os::Mutex> mutex;
  utility::scoped_ptr<os::Cond> cond;

  cq() {
    mutex.reset(os::Mutex::Create());
    cond.reset(os::Cond::Create());
  }
}cq;


int32_t cirq_create(cirq_handle *hndl, uint32_t size) {
  cq * obj = new cq();
  if (!obj)
    return -1;

  obj->max_size = size;

  *hndl = obj;
  return 0;
}
int32_t cirq_destory(cirq_handle hndl) {
  cq * obj = static_cast<cq *>(hndl);
  CHECK(obj);
  delete obj;
  return 0;
}

int32_t cirq_enqueue(cirq_handle hndl, void *data) {
  int32_t ret = -1;
  cq * obj = static_cast<cq *>(hndl);
  obj->mutex->lock();
  if (obj->q.size() >= obj->max_size)
    goto _out;

  obj->q.push_back(data);
  ret = 0;
_out:
  obj->mutex->unlock();
  if (0 == ret)
    obj->cond->signal();
  return ret;
}
int32_t cirq_dequeue(cirq_handle hndl, void **data, uint64_t ms) {
  int32_t ret = -1;
  cq * obj = static_cast<cq *>(hndl);
  obj->mutex->lock();
  if (obj->q.size() <= 0 && ms > 0) {
    obj->cond->wait_timeout(obj->mutex.get(), ms);
  }

  if (obj->q.size() <= 0) {
    goto _out;
  }

  *data = obj->q.front();
  obj->q.pop_front();
  ret = 0;
_out:
  obj->mutex->unlock();
  return ret;
}

int32_t cirq_empty(cirq_handle hndl) {
  int32_t ret = 0;
  cq * obj = static_cast<cq *>(hndl);
  obj->mutex->lock();
  if (obj->q.size() <= 0) {
    ret = 1;
  }
  obj->mutex->unlock();
  return ret;
}
int32_t cirq_full(cirq_handle hndl) {
  int32_t ret = 0;
  cq * obj = static_cast<cq *>(hndl);
  obj->mutex->lock();
  if (obj->q.size() == obj->max_size) {
    ret = 1;
  }
  obj->mutex->unlock();
  return ret;
}

#ifdef __cplusplus
}
#endif
