## Concurrent Linked Queue

### Overview

A queue that allows concurrent access.

### Design

The concurrent queue is made up from a collection sub-queues - each of which
is able to concurrently process an ``add`` and a ``remove`` operation at the 
same time. 

If you create a queue with a high level of concurrency, you should not  
expect to receive items in a strictly first-in-first-out order. Reasons for 
this to happen include the possibility of a thread being suspended during a 
``remove`` operation and the next ``remove`` operation takes the next 
sub-queue's value and returns before the first thread resumes.

### Initialization

To use the concurrent queue you will need an instance of ``struct
octopus_concurrent_linked_queue``.

```c
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
```

### Invalidation

Invalidated ``struct octopus_concurrent_linked_queue`` instances have their 
contents released. You may optionally provide an on-destroy callback to perform
cleanup on the stored types.
