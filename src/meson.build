gis_sources = [
  'main.c',
  'gis-application.c',
  'gis-window.c',
]

gis_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
]

gis_sources += gnome.compile_resources('gis-resources',
  'gis.gresource.xml',
  c_name: 'gis'
)

executable('gis', gis_sources,
  dependencies: gis_deps,
       install: true,
)
