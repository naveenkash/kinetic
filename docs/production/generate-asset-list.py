import os,re,glob,datetime
ROOT='/Users/naveenkashyap/Documents/Projects/Kinetic/docs'
def title_and_line(path):
    title=None; oneline=''
    with open(path,encoding='utf-8') as f:
        lines=[l.rstrip('\n') for l in f]
    for i,l in enumerate(lines):
        if l.startswith('# ') and title is None:
            title=l[2:].strip(); continue
        if title and l.strip() and not l.startswith('#') and not l.startswith('|') and not l.startswith('_') and not l.startswith('```'):
            oneline=re.sub(r'\*\*|`','',l.strip())
            oneline=re.sub(r'^One-line:\s*','',oneline)
            break
    if title is None: title=os.path.basename(path)
    return title, (oneline[:160]+'…' if len(oneline)>160 else oneline)
def words(path):
    return sum(len(l.split()) for l in open(path,encoding='utf-8'))
groups=[
 ('bible','Bible (canonical world, factions, arenas, progression, mechanics)'),
 ('characters/harnesses','Harness classes'),('characters/faces','Faces (named characters)'),
 ('characters/equipment','Equipment'),('characters/npcs','NPCs'),
 ('objects/props-light','Objects: Light props'),('objects/props-medium','Objects: Medium props'),
 ('objects/volatiles','Objects: Volatiles'),('objects/heavy','Objects: Heavy'),
 ('objects/structures','Objects: Structures'),('objects/rated','Objects: Rated loads'),
 ('objects/kinematics','Objects: Kinematics'),('objects/environment','Objects: Environment and world systems'),
 ('arenas/ballast','Arena: Ballast'),('arenas/the-yard','Arena: The Yard'),('arenas/hangtown','Arena: Hangtown'),('arenas/the-anchor','Arena: The Anchor'),
 ('systems','Systems (UI, audio, VFX, animation, camera, modes, onboarding)'),
 ('tech','Technical architecture (UE5)'),('production','Production'),
]
out=['# KINETIC — Master Asset and Document List','',f'_Generated {datetime.date.today()} from docs/. Regenerate with python3 docs/production/generate-asset-list.py after any change. Every row is one file; every object, character, and arena has exactly one._','']
total_files=0; total_words=0
summary=[]
for folder,label in groups:
    files=sorted(glob.glob(os.path.join(ROOT,folder,'*.md')))
    if not files: continue
    w=sum(words(f) for f in files); total_files+=len(files); total_words+=w
    summary.append((label,len(files),w))
    out.append(f'## {label}'); out.append(''); out.append(f'_{len(files)} files, about {w:,} words._'); out.append('')
    out.append('| File | Title | Summary |'); out.append('|---|---|---|')
    for f in files:
        rel=os.path.relpath(f,ROOT); t,o=title_and_line(f)
        out.append(f'| [{os.path.basename(f)}]({rel}) | {t} | {o} |')
    out.append('')
# index files at folder roots
extra=sorted(glob.glob(os.path.join(ROOT,'*/README*.md'))+glob.glob(os.path.join(ROOT,'arenas/README.md')))
out.append('## Folder indexes'); out.append(''); out.append('| File | Title |'); out.append('|---|---|')
for f in sorted(set(extra)):
    rel=os.path.relpath(f,ROOT); t,_=title_and_line(f); out.append(f'| [{rel}]({rel}) | {t} |'); total_files+=1; total_words+=words(f)
out.append('')
out.insert(4,'## Totals'); out.insert(5,''); out.insert(6,'| Section | Files | Words |'); out.insert(7,'|---|---:|---:|')
i=8
for label,n,w in summary:
    out.insert(i,f'| {label} | {n} | {w:,} |'); i+=1
out.insert(i,f'| **Total (with folder indexes)** | **{total_files}** | **{total_words:,}** |'); out.insert(i+1,'')
open(os.path.join(ROOT,'production','asset-master-list.md'),'w',encoding='utf-8').write('\n'.join(out))
print(total_files,total_words)
