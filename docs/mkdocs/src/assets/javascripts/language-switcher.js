// --------------------------------------------------------------------------------
// Copyright (c) 2025 Huawei Technologies Co., Ltd.
// This program is free software, you can redistribute it and/or modify it under the terms and conditions of
// CANN Open Software License Agreement Version 2.0 (the "License").
// Please refer to the License for details. You may not use this file except in compliance with the License.
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
// See LICENSE in the root of the software repository for the full text of the License.
// --------------------------------------------------------------------------------

// Simple language switcher without i18n plugin
(function() {
    'use strict';

    // LocalStorage 键
    const LANG_PREFERENCE_KEY = 'preferred_language';
    
    // 获取用户偏好语言
    function getPreferredLanguage() {
        return localStorage.getItem(LANG_PREFERENCE_KEY) || 'en';
    }
    
    // 设置用户偏好语言
    function setPreferredLanguage(lang) {
        localStorage.setItem(LANG_PREFERENCE_KEY, lang);
    }
    
    // 检测当前页面语言
    function getCurrentLanguage() {
        const path = window.location.pathname;
        if (path.includes('_zh/') || path.endsWith('_zh.html') || path.includes('_zh/index.html')) {
            return 'zh';
        }
        return 'en';
    }
    
    // 获取切换后的 URL
    function getAlternateUrl(targetLang) {
        let currentPath = window.location.pathname;
        const currentLang = getCurrentLanguage();
        
        if (currentLang === targetLang) {
            return currentPath;
        }
        
        if (targetLang === 'zh') {
            // 切换到中文
            // / → /index_zh/
            // /manual/ → /manual/index_zh/
            // /kernels/ → /kernels/README_zh/
            // /docs/ → /docs/README_zh/
            // /manual/01-overview/ → /manual/01-overview_zh/
            
            // 特殊处理：根目录首页
            if (currentPath === '/' || currentPath === '/index.html') {
                return '/index_zh/';
            }
            
            // 移除末尾的斜杠
            if (currentPath.endsWith('/')) {
                currentPath = currentPath.slice(0, -1);
            }
            
            // 特殊处理：manual 目录的 index
            if (currentPath === '/manual' || currentPath === '/manual/index.html') {
                return '/manual/index_zh/';
            }
            
            // 检查是否是 README 页面（通过检查路径模式）
            // 常见的 README 目录：docs, kernels, tests, demos, include, scripts 等
            const readmeDirs = [
                'docs', 'kernels', 'tests', 'demos', 'include', 'scripts', 
                'machine', 'isa', 'ir', 'coding', 'grammar', 'cmake',
                'manual/a2a3', 'manual/a5', 'baseline', 'torch_jit',
                'custom', 'package', 'npu', 'a2a3', 'a5', 'kirin9030',
                'pto', 'flash_atten', 'gemm_performance', 'topk',
                'matmul_mxfp4_performance', 'matmul_mxfp8_performance',
                'add', 'gemm_basic', 'script'
            ];
            
            // 检查路径是否以这些目录结尾（表示是 README）
            const parts = currentPath.split('/').filter(p => p);
            const lastPart = parts[parts.length - 1];
            
            // 如果最后一部分匹配 README 目录，添加 README_zh
            if (readmeDirs.includes(lastPart)) {
                return currentPath + '/README_zh/';
            }
            
            // 检查是否是多级目录的 README（如 kernels/manual/a2a3/gemm_performance）
            const lastTwoParts = parts.slice(-2).join('/');
            const lastThreeParts = parts.slice(-3).join('/');
            const lastFourParts = parts.slice(-4).join('/');
            
            if (readmeDirs.includes(lastTwoParts) || 
                readmeDirs.includes(lastThreeParts) ||
                readmeDirs.includes(lastFourParts)) {
                return currentPath + '/README_zh/';
            }
            
            // 普通页面
            return currentPath + '_zh/';
        } else {
            // 切换到英文
            // /index_zh/ → /
            // /manual/index_zh/ → /manual/
            // /kernels/README_zh/ → /kernels/
            // /manual/01-overview_zh/ → /manual/01-overview/
            
            if (currentPath === '/index_zh/' || currentPath === '/index_zh/index.html') {
                return '/';
            }
            
            if (currentPath === '/manual/index_zh/' || currentPath === '/manual/index_zh/index.html') {
                return '/manual/';
            }
            
            return currentPath.replace(/\/README_zh\/$/, '/').replace(/\/index_zh\/$/, '/').replace(/_zh\/$/, '/');
        }
    }
    
    // 创建语言切换器 UI
    function createLanguageSwitcher() {
        const preferredLang = getPreferredLanguage();
        
        const container = document.createElement('div');
        container.id = 'language-switcher';
        container.style.cssText = `
            position: fixed;
            top: 10px;
            right: 10px;
            z-index: 1000;
            background: white;
            border: 1px solid #ccc;
            border-radius: 4px;
            padding: 5px 10px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            font-family: sans-serif;
            font-size: 14px;
        `;
        
        const languages = [
            { code: 'en', name: 'English', flag: '🇬🇧' },
            { code: 'zh', name: '中文', flag: '🇨🇳' }
        ];
        
        languages.forEach((lang, index) => {
            if (index > 0) {
                const separator = document.createElement('span');
                separator.textContent = ' | ';
                separator.style.color = '#999';
                container.appendChild(separator);
            }
            
            const link = document.createElement('a');
            link.href = '#';
            link.textContent = `${lang.flag} ${lang.name}`;
            link.style.cssText = `
                text-decoration: none;
                color: ${preferredLang === lang.code ? '#2980b9' : '#333'};
                font-weight: ${preferredLang === lang.code ? 'bold' : 'normal'};
                cursor: pointer;
            `;
            
            link.addEventListener('click', function(e) {
                e.preventDefault();
                
                // 保存语言偏好
                setPreferredLanguage(lang.code);
                
                // 更新 UI
                languages.forEach(l => {
                    const links = container.querySelectorAll('a');
                    links.forEach(link => {
                        if (link.textContent.includes(l.name)) {
                            link.style.color = l.code === lang.code ? '#2980b9' : '#333';
                            link.style.fontWeight = l.code === lang.code ? 'bold' : 'normal';
                        }
                    });
                });
                
                // 立即翻译导航栏
                if (lang.code === 'zh') {
                    if (typeof window.translateNavigation === 'function') {
                        window.translateNavigation('zh');
                    }
                } else {
                    if (typeof window.restoreEnglishNavigation === 'function') {
                        window.restoreEnglishNavigation();
                    }
                }
                
                // 尝试跳转到对应语言版本
                const targetUrl = getAlternateUrl(lang.code);
                if (targetUrl !== window.location.pathname) {
                    // 直接跳转，不检查页面是否存在
                    console.log('Switching to:', targetUrl);
                    window.location.href = targetUrl;
                } else {
                    // 已经在目标语言页面，只需要更新横幅
                    if (lang.code === 'zh') {
                        showChineseBanner();
                    } else {
                        removeChineseBanner();
                    }
                }
            });
            
            link.addEventListener('mouseenter', function() {
                if (preferredLang !== lang.code) {
                    this.style.color = '#2980b9';
                }
            });
            
            link.addEventListener('mouseleave', function() {
                if (preferredLang !== lang.code) {
                    this.style.color = '#333';
                }
            });
            
            container.appendChild(link);
        });
        
        document.body.appendChild(container);
    }
    
    // 显示中文横幅（当前在中文页面）
    function showChineseBanner() {
        removeChineseBanner();
        
        const banner = document.createElement('div');
        banner.id = 'chinese-banner';
        banner.style.cssText = `
            background: #e8f4f8;
            border-bottom: 2px solid #2980b9;
            padding: 10px 20px;
            text-align: center;
            font-family: sans-serif;
            font-size: 14px;
            color: #2c3e50;
        `;
        banner.textContent = '🇨🇳 您正在浏览中文版本';
        
        document.body.insertBefore(banner, document.body.firstChild);
    }
    
    // 显示"暂无中文版本"横幅
    function showNoChineseVersionBanner() {
        removeChineseBanner();
        
        const banner = document.createElement('div');
        banner.id = 'chinese-banner';
        banner.style.cssText = `
            background: #fff3cd;
            border-bottom: 2px solid #ffc107;
            padding: 10px 20px;
            text-align: center;
            font-family: sans-serif;
            font-size: 14px;
            color: #856404;
        `;
        banner.innerHTML = '⚠️ 此页面暂无中文版本，显示英文内容 | <a href="#" style="color: #856404; text-decoration: underline;">切换回英文</a>';
        
        banner.querySelector('a').addEventListener('click', function(e) {
            e.preventDefault();
            setPreferredLanguage('en');
            removeChineseBanner();
            restoreNavigation();
            location.reload();
        });
        
        document.body.insertBefore(banner, document.body.firstChild);
    }
    
    // 移除横幅
    function removeChineseBanner() {
        const banner = document.getElementById('chinese-banner');
        if (banner) {
            banner.remove();
        }
    }
    
    // 初始化
    function init() {
        createLanguageSwitcher();
        
        const preferredLang = getPreferredLanguage();
        const currentLang = getCurrentLanguage();
        
        console.log('Initializing language switcher...');
        console.log('Preferred language:', preferredLang);
        console.log('Current page language:', currentLang);
        
        // 如果用户偏好中文，翻译导航栏
        if (preferredLang === 'zh') {
            if (typeof window.translateNavigation === 'function') {
                window.translateNavigation('zh');
            }
            
            // 如果当前是中文页面，显示中文横幅
            if (currentLang === 'zh') {
                showChineseBanner();
            } else {
                // 当前是英文页面，但用户偏好中文，显示提示
                showNoChineseVersionBanner();
            }
        } else {
            // 用户偏好英文
            if (currentLang === 'zh') {
                // 当前在中文页面，但用户偏好英文，可能是直接访问的中文链接
                showChineseBanner();
            }
        }
    }
    
    // 页面加载完成后初始化
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();

