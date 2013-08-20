![CorTeX-Peripheral](https://raw.github.com/dginev/CorTeX/master/public/img/logo.jpg) Peripheral
=================

Peripheral management for the CorTeX system:
 * Spawner service for available CorTeX Services
 * Infrastructure for quick development of Perl services for CorTeX, automated by the spawner service.
 * Default CorTeX services - TeX to (X)HTML conversion flavours, example analysis and aggregation services.

#### Quick Setup

Painless setup for CorTeX processing on any machine (assuming all [prerequisites](./INSTALL.md) are installed):

```shell
git clone https://github.com/dginev/CorTeX-Peripheral
cd CorTeX-Peripheral
perl Makefile.PL ; make ; make test ; sudo make install
sudo service cortex-peripheral start
```

#### Adding new services

Installation of user-supplied Perl services subclassing ```CorTeX::Service```:

```
cp my_perl_service_v0_1.pm /path/to/CorTeX-Peripheral/lib/CorTeX/Service
```

Remember to rerun the Makefile incantations when you add a new service to ```lib/CorTeX/Service/```:

```perl Makefile.PL ; make ; sudo make install```

and to then restart the spawner service via:

```sudo service cortex-peripheral restart```

#### Examples

A few example services are available in this repository, such as a [basic analysis service](lib/CorTeX/Service/mock_spotter_v0_1.pm) counting the already tokenized words and sentences in a document.
