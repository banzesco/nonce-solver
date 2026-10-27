import os, subprocess, time, sys
job=os.environ['JOB']; secs=int(os.environ.get('SECS','780')); idx=int(os.environ['IDX'])
import glob, gzip
parts=sorted(glob.glob('jobs/%s/part*'%job))
cnf='job.cnf'
open(cnf,'wb').write(gzip.decompress(b''.join(open(p,'rb').read() for p in parts)))
lines=open(cnf).read().split('\n')
cls=[]; nv=0
for l in lines:
    l=l.strip()
    if not l or l[0] in 'c%': continue
    if l[0]=='p': nv=int(l.split()[2]); continue
    if nv==0 and len(l.split())==2: nv=int(l.split()[0]); continue
    c=[int(x) for x in l.split() if x!='0']
    if c: cls.append(c)
k=len(cls[0])
cbs={3:['2.5','2.3','2.6','2.4'],4:['3.0','2.8','3.1','2.9'],5:['5.4','5.0','5.6','5.2']}.get(k,['5.4','5.0','5.6','5.2'])
ncpu=os.cpu_count() or 2
procs=[]
if k!=5:
    open('cdcl.py','w').write('''
import sys,random,threading
from pysat.solvers import Solver
seed=int(sys.argv[1]); secs=int(sys.argv[2])
L=[l.split() for l in open('job.cnf').read().strip().split(chr(10))]
nv=int(L[0][0]); cls=[[int(x) for x in l if x!='0'] for l in L[1:] if l]
random.seed(seed)
S=Solver(name='cadical153' if seed%2==0 else 'glucose4',bootstrap_with=cls)
if seed%3: S.set_phases([v if random.random()<0.5 else -v for v in range(1,nv+1)])
t=threading.Timer(secs,lambda:S.interrupt()); t.start()
if S.solve_limited(expect_interrupt=True):
    m=S.get_model(); print(''.join('1' if m[i]>0 else '0' for i in range(nv)))
t.cancel()
''')
for j in range(ncpu):
    f=open('out%d.txt'%j,'w')
    if k!=5:
        procs.append(subprocess.Popen(['timeout',str(secs+5),'python3','cdcl.py',str(idx*1000+j),str(secs)],stdout=f,stderr=subprocess.DEVNULL)); continue
    procs.append(subprocess.Popen(['timeout',str(secs),'./ps',cbs[j%len(cbs)],'100000000000','0','best%d.txt'%j,str(idx*1000+j)],stdin=open(cnf),stdout=f,stderr=subprocess.DEVNULL))
def ok(a):
    if len(a)!=nv: return False
    return all(any((a[abs(x)-1]=='1')==(x>0) for x in c) for c in cls)
sol=None; t0=time.time()
def remote_done():
    r=subprocess.run(['git','ls-remote','origin','refs/heads/sol-'+job],capture_output=True,text=True).stdout
    return bool(r.strip())
last=0
while not sol and any(p.poll() is None for p in procs):
    for j in range(ncpu):
        try:
            a=open('out%d.txt'%j).read().strip()
            if a and ok(a): sol=a
        except Exception: pass
    if time.time()-last>20:
        last=time.time()
        if remote_done(): print('other runner solved'); break
    time.sleep(1)
for p in procs: p.kill()
if not sol:
    for j in range(ncpu):
        a=open('out%d.txt'%j).read().strip()
        if a and ok(a): sol=a
if sol:
    print('SOLVED by',idx,'in',int(time.time()-t0),'s')
    subprocess.run(['git','config','user.email','bot@users.noreply.github.com']); subprocess.run(['git','config','user.name','solver'])
    subprocess.run(['git','checkout','-b','sol-'+job])
    open('sol.txt','w').write(sol+'\n')
    subprocess.run(['git','add','sol.txt']); subprocess.run(['git','commit','-m','sol '+job])
    r=subprocess.run(['git','push','origin','sol-'+job]); print('push',r.returncode)
else:
    bests=[]
    for j in range(ncpu):
        try: bests.append(int(open('best%d.txt'%j).read().split()[1]))
        except Exception: pass
    print('no solution; best unsat', min(bests) if bests else None)
