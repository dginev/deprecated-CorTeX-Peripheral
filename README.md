![CorTeX-Peripheral](https://raw.github.com/dginev/CorTeX/master/public/img/logo.jpg) Peripheral
=================

Peripheral management for the CorTeX system:
 * Spawner service for available CorTeX Services
 * Infrastructure for quick development of Perl services for CorTeX, automated by the spawner service.
 * Default CorTeX services - TeX to (X)HTML conversion flavours, example analysis and aggregation services.

Painless setup for CorTeX processing on any machine:
```shell
git clone https://github.com/dginev/CorTeX-Peripheral
cd CorTeX-Peripheral
perl cortex-spawner IP-of-Gearman-server
```

Installation of user-supplied Perl services subclassing ```CorTeX::Service```:
```
cp my_perl_service_v0_1.pm /path/to/CorTeX-Peripheral/lib/CorTeX/Service
```

A few example services are available in this repository, such as a [basic example analysis service](lib/CorTeX/Service/mock_spotter_v0_1.pm).
