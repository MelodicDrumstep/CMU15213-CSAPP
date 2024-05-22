这个 Lab 是写 web proxy.

## What is Web Proxy

Web proxy 是 Web 浏览器和服务器的中间层，有了它之后， 浏览器不是直接从服务器中拿到 web page, 而是先向 web proxy 发送请求， web proxy 再把请求推送给服务器。待服务器响应后， web proxy 又再把 web page 返回给浏览器。

## Web Proxy 的用途

+ 构建防火墙

+ 保证匿名性

可以在 web proxy 这一层去除请求中包含的隐私信息。

+ 缓存

可以把一些 page 缓存在这里， 命中之后就无需打扰服务器。

## 任务

这次的任务是用 C 写一个基于 HTTP 协议的用于缓存 web object 的 web proxy。

第一部分会构建 web proxy， 使其能够接受请求， 读取请求， 转发请求。第一部分主要是学习 HTTP operation 以及 C 的 socket 接口。

第二部分会增添并发组件， 使其可以并发处理请求。

第三部分会增添 cache。

