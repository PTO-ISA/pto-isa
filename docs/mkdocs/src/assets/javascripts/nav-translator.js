// --------------------------------------------------------------------------------
// Copyright (c) 2025 Huawei Technologies Co., Ltd.
// This program is free software, you can redistribute it and/or modify it under the terms and conditions of
// CANN Open Software License Agreement Version 2.0 (the "License").
// Please refer to the License for details. You may not use this file except in compliance with the License.
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
// See LICENSE in the root of the software repository for the full text of the License.
// --------------------------------------------------------------------------------

// 导航栏中英文映射表
const NAV_TRANSLATIONS = {
    // 顶级导航
    'Home': '首页',
    'PTO Virtual ISA Manual': 'PTO 虚拟 ISA 手册',
    'Programming Model': '编程模型',
    'ISA Reference': 'ISA 参考',
    'Machine Model': '机器模型',
    'Examples': '示例',
    'Documentation': '文档',
    'Full Index': '完整索引',
    
    // PTO Virtual ISA Manual 子项
    'Preface': '前言',
    'Overview': '概述',
    'Execution Model': '执行模型',
    'State and Types': '状态与类型',
    'Tiles and GlobalTensor': 'Tile 与 GlobalTensor',
    'Synchronization': '同步',
    'PTO Assembly (PTO-AS)': 'PTO 汇编 (PTO-AS)',
    'Instruction Set (overview)': '指令集（概述）',
    'Programming Guide': '编程指南',
    'Virtual ISA and IR': '虚拟 ISA 与 IR',
    'Bytecode and Toolchain': '字节码与工具链',
    'Memory Ordering and Consistency': '内存顺序与一致性',
    'Backend Profiles and Conformance': '后端配置与一致性',
    'Glossary': '术语表',
    'Instruction Contract Template': '指令契约模板',
    'Diagnostics Taxonomy': '诊断分类',
    'Instruction Family Matrix': '指令族矩阵',
    
    // Programming Model 子项
    'Tile': 'Tile',
    'GlobalTensor': 'GlobalTensor',
    'Scalar': 'Scalar',
    'Event': 'Event',
    'Tutorial': '教程',
    'Tutorials': '教程集',
    'Example: Vec Add': '示例：向量加法',
    'Example: Row Softmax': '示例：行 Softmax',
    'Example: GEMM': '示例：GEMM',
    'Optimization': '优化',
    'Debugging': '调试',
    
    // ISA Reference 子项
    'ISA index': 'ISA 索引',
    'PTO IR ops index': 'PTO IR 操作索引',
    'ISA conventions': 'ISA 约定',
    'PTO ISA table': 'PTO ISA 表',
    'Intrinsics header': '内建函数头文件',
    'PTO-AS spec': 'PTO-AS 规范',
    'Grammar conventions': '语法约定',
    'Grammar index': '语法索引',
    
    // Machine Model 子项
    'Abstract machine': '抽象机器',
    'Machine index': '机器索引',
    
    // Examples 子项
    'Kernels index': '算子索引',
    'GEMM performance kernel': 'GEMM 性能算子',
    'Flash Attention kernel': 'Flash Attention 算子',
    'Baseline add demo': '基础加法示例',
    'Baseline GEMM demo': '基础 GEMM 示例',
    'Tests index': '测试索引',
    'Test scripts': '测试脚本',
    
    // Documentation 子项
    'Docs index': '文档索引',
    'Build this site': '构建本站点',
    'Root README': '根目录 README'
};

// 导航栏翻译函数
function translateNavigation(targetLang) {
    if (targetLang !== 'zh') {
        return; // 只翻译为中文
    }
    
    // 查找所有导航链接
    const navLinks = document.querySelectorAll('.wy-menu-vertical a, nav a, .toctree-l1 > a, .toctree-l2 > a');
    
    navLinks.forEach(link => {
        const originalText = link.textContent.trim();
        
        // 跳过空文本
        if (!originalText) return;
        
        // 翻译文本
        if (NAV_TRANSLATIONS[originalText]) {
            link.textContent = NAV_TRANSLATIONS[originalText];
        }
        
        // 修改链接指向中文版本
        // 保存原始 href（如果还没保存）
        if (!link.hasAttribute('data-original-href')) {
            link.setAttribute('data-original-href', link.getAttribute('href'));
        }
        
        // 使用原始 href 进行转换
        const href = link.getAttribute('data-original-href');
        if (href && !href.startsWith('#') && !href.startsWith('http')) {
            // 转换链接到中文版本
            let newHref = href;
            
            // 保存相对路径前缀
            const relativePrefix = href.match(/^(\.\.\/)+/);
            
            // 标记是否是首页链接
            let isHomePage = false;
            
            // 特殊处理：根目录首页（支持相对路径和绝对路径）
            if (newHref === '..' || newHref === '../' || newHref === '../..' || newHref === '../../' || 
                newHref === '../../..' || newHref === '../../../' ||
                newHref === '.' || newHref === './' || 
                newHref === '/' || newHref === '/.' || newHref === '/index.html' ||
                newHref === 'index.html') {
                newHref = '/index_zh/';
                isHomePage = true;
            } else {
                // 移除开头的 ../
                newHref = newHref.replace(/^(\.\.\/)+/, '');
                
                // 移除末尾的斜杠进行处理
                if (newHref.endsWith('/')) {
                    newHref = newHref.slice(0, -1);
                }
                
                // 再次检查是否是首页
                if (newHref === '' || newHref === 'index.html') {
                    newHref = '/index_zh/';
                    isHomePage = true;
                }
            }
            
            if (!isHomePage) {
                // 特殊处理：manual 目录的 index
                if (newHref === 'manual' || newHref === 'manual/index.html') {
                    newHref = 'manual/index_zh/';
                }
                // 检查是否是 README 页面
                else {
                    const parts = newHref.split('/').filter(p => p);
                    const lastPart = parts[parts.length - 1];
                    
                    // README 目录列表（这些目录下的页面使用 README.md）
                    const readmeDirs = [
                        'docs', 'kernels', 'tests', 'demos', 'include', 'scripts', 
                        'machine', 'isa', 'ir', 'coding', 'grammar', 'cmake',
                        'baseline', 'torch_jit', 'custom', 'package', 
                        'npu', 'a2a3', 'a5', 'kirin9030', 'pto', 
                        'flash_atten', 'gemm_performance', 'topk',
                        'matmul_mxfp4_performance', 'matmul_mxfp8_performance',
                        'add', 'gemm_basic', 'script', 'tutorials', 'comm',
                        'reference'
                    ];
                    
                    // 只检查最后一部分是否是 README 目录
                    const isReadmeDir = readmeDirs.includes(lastPart);
                    
                    if (isReadmeDir) {
                        // README 页面
                        newHref = newHref + '/README_zh/';
                    } else {
                        // 普通页面
                        newHref = newHref + '_zh/';
                    }
                }
            }
            
            // 恢复相对路径前缀（仅对非首页链接）
            if (relativePrefix && !isHomePage) {
                newHref = relativePrefix[0] + newHref;
            }
            
            link.setAttribute('href', newHref);
        }
    });
    
    // 翻译导航栏的大标题（caption-text）
    const captionTexts = document.querySelectorAll('.caption-text');
    captionTexts.forEach(caption => {
        const originalText = caption.textContent.trim();
        
        if (!originalText) return;
        
        if (NAV_TRANSLATIONS[originalText]) {
            caption.textContent = NAV_TRANSLATIONS[originalText];
        }
    });
    
    // 翻译站点标题
    const siteTitle = document.querySelector('.wy-side-nav-search a, .navbar-brand');
    if (siteTitle && siteTitle.textContent.includes('PTO Virtual ISA')) {
        siteTitle.textContent = 'PTO 虚拟 ISA 架构手册';
    }
}

// 恢复英文导航
function restoreEnglishNavigation() {
    // 英文是默认的，刷新页面即可恢复
}

// 导出函数供语言切换器使用
window.translateNavigation = translateNavigation;
window.restoreEnglishNavigation = restoreEnglishNavigation;

