"""Compare the firmware with the professor's ZIP and inventory C definitions.

Optional analysis dependencies (not used to build the firmware):
    python -m pip install -r scripts/requirements-analysis.txt
Run from any folder. Reports are reproducible and contain no absolute paths.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]
PROJECT = ROOT / 'Game_Start2_STM32F411CE'
ARCHIVE = ROOT / 'referencias/Game_Start2_STM32F411CE.zip'
OUTPUT = ROOT / 'docs/comparacao'
EXCLUDED = {'.git', '.settings', 'Debug', 'Release', 'build', '__MACOSX'}


def included(name):
    path = Path(name)
    return not (set(path.parts) & EXCLUDED) and path.suffix != '.launch'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def group(name):
    if name.startswith('Drivers/STM32F4xx_HAL_Driver/'):
        return 'HAL STM32'
    if name.startswith('Drivers/CMSIS/'):
        return 'CMSIS e dispositivo'
    if name.startswith('Middlewares/'):
        return 'FreeRTOS e adaptador CMSIS-RTOS v2'
    if name.startswith('Legacy/'):
        return 'Exemplos preservados'
    if name.startswith('Core/'):
        return 'Core: aplicacao, suporte e dados'
    return 'Projeto e configuracao'


def walk(node):
    yield node
    for child in node.named_children:
        yield from walk(child)


def source_text(node, data):
    return data[node.start_byte:node.end_byte].decode('utf-8', errors='replace')


def declarator_name(node, data):
    while node is not None:
        if node.type in ('identifier', 'type_identifier', 'field_identifier'):
            return source_text(node, data)
        node = node.child_by_field_name('declarator')
    return '(macro ou declarador nao resolvido)'


def inventory(parser, name, data):
    tree = parser.parse(data)
    functions, types, includes = [], [], []
    for node in walk(tree.root_node):
        if node.type == 'function_definition':
            declaration = node.child_by_field_name('declarator')
            body = node.child_by_field_name('body')
            signature = data[node.start_byte:body.start_byte].decode('utf-8', errors='replace')
            entry = {'name': declarator_name(declaration, data),
                     'line': node.start_point.row + 1,
                     'end_line': node.end_point.row + 1,
                     'signature': ' '.join(signature.split())}
            if name.startswith(('Core/', 'Legacy/')):
                entry['calls'] = sorted({source_text(call.child_by_field_name('function'), data)
                                         for call in walk(body) if call.type == 'call_expression'})
            functions.append(entry)
        elif node.type == 'type_definition':
            types.append({'line': node.start_point.row + 1,
                          'declaration': source_text(node, data)})
        elif node.type == 'preproc_include':
            includes.append(source_text(node, data).strip())
    return {'functions': functions, 'typedefs': types, 'includes': includes,
            'syntax_errors': [{'line': n.start_point.row + 1, 'kind': n.type}
                              for n in walk(tree.root_node) if n.type == 'ERROR' or n.is_missing]}


def main():
    args = argparse.ArgumentParser(description=__doc__)
    args.add_argument('--check', action='store_true', help='Fail if checked-in reports are stale')
    args.add_argument('--module-path', type=Path, help='Directory containing the optional Python packages')
    options = args.parse_args()
    if options.module_path:
        sys.path.insert(0, str(options.module_path.resolve()))
    try:
        from tree_sitter import Language, Parser
        import tree_sitter_c
    except ImportError:
        args.error('Install scripts/requirements-analysis.txt to generate the C inventory.')
    parser = Parser(Language(tree_sitter_c.language()))
    original = {}
    with zipfile.ZipFile(ARCHIVE) as archive:
        for item in archive.infolist():
            if item.is_dir():
                continue
            name = item.filename.replace('\\', '/')
            prefix = 'Game_Start2_STM32F411CE/'
            if name.startswith(prefix):
                name = name[len(prefix):]
            if included(name):
                original[name] = archive.read(item)
    current = {p.relative_to(PROJECT).as_posix(): p.read_bytes()
               for p in PROJECT.rglob('*') if p.is_file() and included(p.relative_to(PROJECT).as_posix())}
    files = []
    consumed = set()
    for name, data in sorted(original.items()):
        target = name
        if target not in current and name.startswith('Core/'):
            candidate = name.replace('Core/', 'Legacy/', 1)
            if candidate in current:
                target = candidate
        new = current.get(target)
        status = 'removed' if new is None else ('unchanged' if data == new else 'modified')
        if target != name:
            status = 'moved-identical' if data == new else 'moved-modified'
        entry = {'original': name, 'current': target if new is not None else None,
                 'group': group(target), 'status': status, 'original_sha256': digest(data)}
        if new is not None:
            entry['current_sha256'] = digest(new)
            consumed.add(target)
        files.append(entry)
    for name in sorted(current.keys() - consumed):
        files.append({'original': None, 'current': name, 'group': group(name),
                      'status': 'added', 'current_sha256': digest(current[name])})
    inventories = {label: {name: inventory(parser, name, data)
                           for name, data in sorted(contents.items()) if Path(name).suffix in ('.c', '.h')}
                   for label, contents in [('original', original), ('current', current)]}
    report = {'archive': ARCHIVE.relative_to(ROOT).as_posix(), 'archive_sha256': digest(ARCHIVE.read_bytes()),
              'scope': 'Firmware files; excludes Debug, Release, .settings, .launch, build and .git. '
                       'Definitions/typedefs are syntactic, before preprocessing; not all are compiled. '
                       'Macro-heavy vendor files can have parser errors, listed per file. No call-graph or safety proof.',
              'counts': dict(sorted(Counter(f['status'] for f in files).items())),
              'files': files, 'inventory': inventories}
    lines = ['# Inventário comparado com a base do professor', '',
             'Gerado por `scripts/compare_professor.py`. Não editar manualmente.', '',
             'Base: `referencias/Game_Start2_STM32F411CE.zip`.',
             f'SHA-256: `{report["archive_sha256"]}`.', '',
             'A comparação de arquivos é byte a byte. Builds, preferências locais e configurações',
             'de lançamento foram excluídos. O JSON inclui hashes, funções, assinaturas, typedefs,',
             'includes e chamadas diretas do Core/Legacy, tanto da base como da versão atual.', '',
             'O inventário sintático não executa o pré-processador: funções condicionais e exemplos',
             'podem aparecer mesmo sem entrar no firmware. Macros de bibliotecas podem produzir',
             'erros de análise; eles ficam identificados no JSON. Não é uma auditoria formal das bibliotecas.', '',
             '## Arquivos', '']
    lines += [f'- `{status}`: {count}.' for status, count in report['counts'].items()]
    lines += ['', '### Alterados, acrescentados ou movidos', '']
    for file in files:
        if file['status'] == 'unchanged':
            continue
        location = file['current'] or file['original']
        moved = f' (antes: `{file["original"]}`)' if file['status'].startswith('moved') else ''
        lines.append(f'- **{file["status"]}**: `{location}`{moved}.')
    lines += ['', '## Bibliotecas e quantidade de definições', '']
    groups = Counter()
    for name, item in inventories['current'].items():
        groups[group(name)] += len(item['functions'])
    lines += [f'- {name}: {count} definições sintáticas de função.' for name, count in sorted(groups.items())]
    lines += ['', '## Funções e tipos do Core e dos exemplos', '',
              'Os números de linha apontam para a versão atual. O arquivo JSON também inventaria',
              'todas as definições reconhecidas nas bibliotecas HAL, CMSIS e FreeRTOS.', '']
    for name, item in inventories['current'].items():
        if not name.startswith(('Core/', 'Legacy/')) or not (item['functions'] or item['typedefs']):
            continue
        lines += [f'### `{name}`', '']
        for fun in item['functions']:
            lines.append(f'- L{fun["line"]}: `{fun["signature"]}`.')
        for alias in item['typedefs']:
            lines.append(f'- L{alias["line"]}, tipo: `{" ".join(alias["declaration"].split())}`.')
        if item['syntax_errors']:
            lines.append('- Atenção: há construções não resolvidas pelo parser; consultar o JSON e o fonte.')
        lines.append('')
    results = {'inventario.json': json.dumps(report, ensure_ascii=False, indent=2) + '\n',
               'INVENTARIO.md': '\n'.join(lines).rstrip() + '\n'}
    OUTPUT.mkdir(parents=True, exist_ok=True)
    for name, content in results.items():
        path = OUTPUT / name
        if options.check:
            if not path.exists() or path.read_text(encoding='utf-8') != content:
                args.error(f'Report needs regeneration: {path}')
        else:
            path.write_text(content, encoding='utf-8', newline='\n')
    print(json.dumps(report['counts'], ensure_ascii=False))
    for label, contents in inventories.items():
        print(f'{label}: {len(contents)} C/header files, '
              f'{sum(len(i["functions"]) for i in contents.values())} function definitions; '
              f'{sum(bool(i["syntax_errors"]) for i in contents.values())} files with parser limitations.')


if __name__ == '__main__':
    main()
