import datetime
import os
import sys

project_dir = sys.argv[1]
conf_file = os.path.join(project_dir, '.bintray.json')
template_file = conf_file + '.in'
curr_date = datetime.date.today().strftime('%Y-%m-%d')


class VarInfo:
  def __init__(self, name, user_defined = '',
               travis='', appveyor='',
               default_value = '<>'):
    self.name = name
    self.user_defined = user_defined
    self.travis = travis
    self.appveyor = appveyor
    if default_value == '<>':
      self.default_value = '<undefined_%s>' % name
    else:
      self.default_value = default_value

  def get(self, ci_type='No'):
    return self._h('No', self._h(ci_type, self.default_value))
    
  def _g(self, v, d):
    if v != '':
      return os.environ.get(v, d)
    return d

  def _h(self, ci_type, d):
    if ci_type == 'No':
      return self._g(self.user_defined, d)
    elif ci_type == 'Travis':
      return self._g(self.travis, d)
    elif ci_type == 'Appveyor':
      return self._g(self.appveyor, d)


project_name_var = VarInfo(
    'project',
    'PROJECT_NAME',
    default_value=os.path.basename(project_dir))
os_name_var = VarInfo(
    'os',
    'OS_NAME', travis='TRAVIS_OS_NAME')
version_var = VarInfo(
    'version',
    'VERSION')
build_no_var = VarInfo(
    'build',
    'BUILD_NO', travis='TRAVIS_BUILD_NUMBER',
    appveyor='APPVEYOR_BUILD_NUMBER')
branch_var = VarInfo(
    'branch',
    'BRANCH', travis='TRAVIS_BRANCH',
    appveyor='APPVEYOR_REPO_BRANCH')
commit_var = VarInfo(
    'commit',
    'COMMIT', travis='TRAVIS_COMMIT',
    appveyor='APPVEYOR_REPO_COMMIT')
date_var = VarInfo(
    'date',
    default_value=curr_date)


var_list = [
    project_name_var,
    os_name_var,
    version_var,
    build_no_var,
    branch_var,
    commit_var,
    date_var]

ci_type = 'No'
if os.environ.get('CI', 'False').lower() == 'true':
  if os.environ.get('TRAVIS', 'False').lower() == 'true':
    ci_type = 'Travis'
  elif os.environ.get('APPVEYOR', 'False').lower() == 'true':
    ci_type = 'Appveyor'


fin = open(template_file)
fout = open(conf_file, 'wt')
sys.stdout.write('Generating %s\n' % conf_file)
for line in fin:
  for v in var_list:
    line = line.replace('{{%s}}' % v.name, v.get(ci_type))
  fout.write(line)
  sys.stdout.write(line)
fin.close()
fout.close()

