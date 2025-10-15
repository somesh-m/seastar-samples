# 🧩 Seastar Samples

> Minimal, focused examples for experimenting with Seastar’s sharded architecture and coroutine-based I/O.  
> Each folder is an independent, buildable sample.

---

## 📁 Samples

### **1️⃣ all_shard_listener**
Shows how each Seastar shard can listen on its own TCP port using  
`server_socket::load_balancing_algorithm::fixed`.

**Highlights**
- Per-shard port binding (base + shard ID)
- Coroutine-based accept loop
- Graceful shutdown using `gate` + `abort_source`

**Run**
```bash
mkdir build && cd build
cmake ..
make
./example --smp 4
```

## 🧑‍💻 Author  
**Somesh Mohan**  
Systems engineer & Seastar enthusiast.  
[LinkedIn](https://in.linkedin.com/in/someshmohan) • [Medium](https://medium.com/@somesh557) • [Github](https://github.com/somesh-m)


