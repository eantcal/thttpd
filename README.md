# thttpd
TinyHttpServer is a lightweight web server implemented in C++11

![TinyHttpServer](https://7bcac53c-a-62cb3a1a-s-sites.googlegroups.com/site/eantcal/tinyhttp2.jpg?attachauth=ANoY7cpMFEDW4qjiNYxyezSyQyarDKg0klCmtkAwoaf2iAgU07JagzbyWL41sFilafSrhPg-U7XVlSuz9AJjVkDJAQ4NYRTcbcWdnqabRLTxVm3cSBaMU2dvYiHpOZPYs71ER-OmozI52HTyFW_VezeyBSuO4a-Tqipo-RXjF6wPVYEszNB46bd-uBHj-wqYrgigVyA3h-a8NeUKgL_AhFv5qClLTP2B3A%3D%3D&attredirects=0&height=174&width=320)

TinyHttpServer is a portable tiny HTTP server implementation written in C++11 (you may compile it using either MS Visual Studio or GNU GCC).

It is capable to serve multiple clients supporting GET method. 
It has been designed mainly for educational purposes for C++ developers that can deal with a non-trivial example of C++11 programming.

It can be compiled for both Windows and Linux operating systems (and maybe others).

## HTTP Protocol
HTTP (Hypertext Transfer Protocol, defined in RFC 2616) is the application protocol used primarily for the delivery of hypertext content on the web. 

The HTTP is transaction-oriented paradigm based, in particular, on the request / response model: the client issues a request to a server that replies with a response. 
TCP sessions used for the transfer of this exchange, are generally established and terminated for each transaction. 
This behavior makes it easy to manage hyperlinks which by their nature allow you to mix content belonging to different sources.
The request / response model can be made more complex in the case in which a non-transparent proxy exists.
A generic HTTP request message is composed of three parts: 
the request method (GET, HEAD, POST, etc ...) followed by a URI and the protocol version, 
a section called header which contains additional information on the method, in the form of attributes (header fields), and finally, if present, 
the body of the message. 
A response consists of a status-line containing a three-digit code that provides the result of the response itself. 
For example, a status line with "HTTP/1.1 200 OK" indicates that the request was successful. 
Header and (optionally) body follow the status line.
The body is a generic octet stream, even if there is a characterization that allows the receiver to properly classify it. 
The header that precedes the body is a sequence of ASCII text lines, each terminated by a couple of the characters CR (0xA) and LF (0xD). 

To summarize a generic HTTP request has the following format:

```
< method > < URI > < version > < CRLF >
< header_field _1 >: < value1 > < CRLF >
< header_field _2 >: < value2 > < CRLF >
...
< header_field _N >: < valueN >< CRLF >< CRLF >
< body ... >
```

Similarly, a generic response is formatted as follows:

```
< version > < status code > < status message > < CRLF >
< header_field _1 >: < value1 >< CRLF >
< header_field _2 >: < value2 >< CRLF >
...
< header_field _N >: < valueN >< CRLF >< CRLF>
< body ... >
```

Note that the last line of the header is identified by a double < CRLF > sequence. 
In practice, the header does not have a predetermined size, but its end and the beginning of the body of the message are identified by this sequence.
The body can either have a predetermined size (a header attribute called Content-Length is specially used), or it can be terminated when the TCP connection is closed.
To classify and treat the data transferred in an appropriate manner, a header attribute named Content-Type is used.

There is an encoding of the types of data sent, described in RFC 1521, which provides a classification of the type of content transferred. 
This classification is called Multimedia Internet Mail Extensions (MIME) because it was originally designed for e-mail messages, and subsequently used for the content of other protocols including HTTP. 

The MIME classification is made by a type, a subtype and optionally a parameter. For example, plain text is normally classified by specifying the attribute 
"Content-Type: text/plain; charset=us-ascii"


![HTTP Server](https://sites.google.com/site/eantcal/archive/tinyhttpserver/tinyhttp.png)

## Thread model
![HTTP Server](https://sites.google.com/site/eantcal/archive/tinyhttpserver/tinyhttp2.png)

## Pseudo code
![HTTP Server](https://sites.google.com/site/eantcal/archive/tinyhttpserver/tinyhttp3.png)

