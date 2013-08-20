## ![CorTeX-Peripheral](https://raw.github.com/dginev/CorTeX/master/public/img/logo.jpg) Peripheral - Installation Instructions

For now, assuming you're running a Debian-based OS

0. Prerequisites

  0.1. Debian packages

  ```sudo apt-get install libanyevent-perl cpanminus```

  0.2. CPAN modules

  ```sudo cpanm AnyEvent::Gearman Unix::Processors```

  0.3. Software suites:
    * [LaTeXML](https://svn.mathweb.org/repos/LaTeXML/branches/arXMLiv/webapp/INSTALL)
    * [LLaMaPUn](https://github.com/dginev/LLaMaPUn)

1. Classic Make-based installation

  ```perl Makefile.PL ; make ; make test ; sudo make install ```

2. Deploy via Linux service:

  **Note:** edit ```bin/cortex-peripheral``` and change the ```GEARMAN_URLS``` field
  in order to point the worker processes to any additional and/or different Gearman servers.

  ```sudo service cortex-peripheral start```


  or, aletnartively, directly starting a spawner process in the foreground:

  ```perl bin/cortex-spawner 127.0.0.1 ```
