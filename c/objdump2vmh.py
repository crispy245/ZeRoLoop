import re
import sys
import fileinput

__hexre = '[0-9a-fA-F]'
__hexvre = __hexre + '+'
__hex8re = __hexre + '{8}'
__secheadre = r'^Contents of section (?P<section>[\.\w]+):$'
__linere = r'^\s*(?P<addr>' + __hexvre + ')' + \
           r'\s*(?P<v0>' + __hex8re + ')?' + \
           r'\s*(?P<v1>' + __hex8re + ')?' + \
           r'\s*(?P<v2>' + __hex8re + ')?' + \
           r'\s*(?P<v3>' + __hex8re + ')?'


def main():
    state = 'init'
    scp = re.compile(__secheadre)
    lp = re.compile(__linere)

    # Print the bootstrap string (optional depending on use case)
    # print(__bootstrapstr)

    for line in fileinput.input():
        line = line.rstrip()

        m = scp.match(line)
        if m:
            state = 'init'
            print('')
            continue

        m = lp.match(line)
        if m:
            if state == 'init':
                print('@{0:x}'.format(int(m.group('addr'), 16)))  # Integer division, jeez took me months to find this error
            state = 'in-section'
            for v in m.group('v0', 'v1', 'v2', 'v3'):
                if v:
                    print(v[6:8] + v[4:6] + v[2:4] + v[0:2])
            continue

if __name__ == '__main__':
    main()