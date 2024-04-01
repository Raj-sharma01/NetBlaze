usage-- \
./main `<Download_location>` `<file1URl>` `<file2Url>` ...


fileURL example-- \
https://nextjs.org/static/blog/next-14/twitter-card.png \
https://download.logo.wine/logo/Node.js/Node.js-Logo.wine.png \
https://www.goodworklabs.com/wp-content/uploads/2016/10/reactjs.png

downloaded file name will be the term after the last "/" \
example for the given URLs-- \
twitter-card.png \
Node.js-Logo.wine. png \
reactjs.png

semaphore is used so only a fixed number (MAX_CONCURRENT_DOWNLOADS) of files can be downloaded at a time  