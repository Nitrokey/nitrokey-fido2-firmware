from logging import basicConfig, INFO, getLogger

basicConfig(format='* %(relativeCreated)6dms %(filename)s:%(lineno)d %(message)s',level=INFO)
log = getLogger('test_solo')
print = log.info