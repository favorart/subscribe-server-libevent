# Publish-Subscribe server

Technosphere.mail.ru 2015

using libevent

**Execute:**

./subs<br>
 + -h/--ip      [server ip-adress]
 + -p/--port    [listening port]
 + -q/--queues  [number of queues]
 
It is a server that has N queues of messages.<br>
Clients can publish messages to its queues<br>
or subscribe to listen messages from queues.

**Protocol:**  

After the client is connected, he can send one of the commands:
 * publish \<num\> "message"
 * subscribe \<num\>

, where \<num\> - number of queue

Servers can reply one of the answers:
 * OK \<num\>
 * Error \<err_text\>
 
