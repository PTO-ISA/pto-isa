// --------------------------------------------------------------------------------
// Copyright (c) 2025 Huawei Technologies Co., Ltd.
// This program is free software, you can redistribute it and/or modify it under the terms and conditions of
// CANN Open Software License Agreement Version 2.0 (the "License").
// Please refer to the License for details. You may not use this file except in compliance with the License.
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
// See LICENSE in the root of the software repository for the full text of the License.
// --------------------------------------------------------------------------------

/**
 * PTO syntax highlighting support for the docs website.
 *
 * Many PTO examples are fenced as `text`, `asm`, or `mlir`, but share one
 * PTO-specific surface: SSA values, `pto.*` ops, `!pto.*` types, and the
 * `ins()/outs()` assembly forms. Register a custom Highlight.js language and
 * relabel matching blocks so they are highlighted consistently.
 */
(function () {
    'use strict';

    function looksLikePtoSnippet(text) {
        return (
            /!pto\.[A-Za-z_][\w.]*/.test(text) ||
            /\bpto\.[A-Za-z_][\w.]*/.test(text) ||
            /\bins\s*\(/.test(text) ||
            /\bouts\s*\(/.test(text) ||
            /^%[A-Za-z_.$][\w.$-]*\s*=/.test(text) ||
            /^#\s*pto\.[A-Za-z_][\w.]*/m.test(text)
        );
    }

    function ptoLanguage(hljs) {
        var IDENT = '[A-Za-z_.$][\\w.$-]*';
        var PTO_IDENT = 'pto\\.[A-Za-z_][\\w.]*';
        var PTO_TYPE = '!pto\\.[A-Za-z_][\\w.]*';

        return {
            name: 'PTO',
            aliases: ['pto', 'pto-as', 'pto-ir'],
            contains: [
                hljs.HASH_COMMENT_MODE,
                hljs.C_BLOCK_COMMENT_MODE,
                hljs.C_LINE_COMMENT_MODE,
                {
                    className: 'keyword',
                    begin: '\\b(ins|outs)\\b'
                },
                {
                    className: 'literal',
                    begin: '\\b(true|false|null)\\b'
                },
                {
                    className: 'number',
                    variants: [
                        { begin: '\\b0x[0-9A-Fa-f]+\\b' },
                        { begin: '\\b\\d+(?:\\.\\d+)?\\b' }
                    ]
                },
                {
                    className: 'variable',
                    begin: '%[A-Za-z_.$][\\w.$-]*'
                },
                {
                    className: 'symbol',
                    begin: '@[A-Za-z_.$][\\w.$-]*(?:\\([^\\n)]*\\))?'
                },
                {
                    className: 'type',
                    begin: PTO_TYPE,
                    relevance: 10
                },
                {
                    className: 'built_in',
                    begin: '\\b(?:Tile|GlobalTensor|Shape|Stride|RecordEvent|TileType|BLayout|SLayout)\\b'
                },
                {
                    className: 'title.function',
                    begin: '\\b(?:' + PTO_IDENT + '|[a-z][a-z0-9_]*|T[A-Z0-9_]+)\\b',
                    relevance: 5
                },
                {
                    className: 'string',
                    begin: '"',
                    end: '"'
                },
                {
                    className: 'punctuation',
                    begin: '->|[:=<>{}()[\\],]'
                }
            ]
        };
    }

    function registerLanguage() {
        if (!window.hljs) {
            return;
        }

        if (!window.hljs.getLanguage('pto')) {
            window.hljs.registerLanguage('pto', ptoLanguage);
        }
    }

    function normalizeBlockLanguage(block) {
        var className = block.className || '';
        var text = block.textContent || '';
        var isCandidate = /\blanguage-(text|asm|mlir)\b/.test(className) || !/\blanguage-/.test(className);

        if (!isCandidate || !looksLikePtoSnippet(text)) {
            return;
        }

        block.className = className.replace(/\blanguage-(text|asm|mlir)\b/g, '').trim();
        block.classList.add('language-pto');
        block.removeAttribute('data-highlighted');
    }

    function normalizeBlocks() {
        document.querySelectorAll('pre code').forEach(normalizeBlockLanguage);
    }

    function init() {
        registerLanguage();
        normalizeBlocks();
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
