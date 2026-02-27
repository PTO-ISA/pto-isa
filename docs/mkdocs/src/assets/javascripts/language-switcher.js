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
    
    console.log('Language switcher loaded');
    
    // Detect current language from filename
    function getCurrentLanguage() {
        const path = window.location.pathname;
        // Check if current page is Chinese version (_zh)
        if (path.includes('_zh')) return 'zh';
        return 'en';
    }
    
    // Get the alternate language URL
    function getAlternateUrl(targetLang) {
        const currentPath = window.location.pathname;
        const currentLang = getCurrentLanguage();
        
        if (currentLang === targetLang) {
            return currentPath;
        }
        
        // Switch between English and Chinese versions
        if (currentLang === 'en' && targetLang === 'zh') {
            // Special handling for index.html (README.md)
            if (currentPath.endsWith('/index.html') || currentPath.endsWith('/')) {
                // Convert /path/index.html or /path/ to /path_zh/
                const basePath = currentPath.replace(/\/(index\.html)?$/, '');
                return basePath + '_zh/';
            }
            // Convert file.html to file_zh.html or file_zh/
            else if (currentPath.endsWith('.html')) {
                const withoutExt = currentPath.replace(/\.html$/, '');
                return withoutExt + '_zh/';
            } else {
                return currentPath + '_zh/';
            }
        } else if (currentLang === 'zh' && targetLang === 'en') {
            // Convert file_zh/ or file_zh.html to file.html or /
            if (currentPath.includes('_zh/')) {
                // /path_zh/ -> /path/ or /path/index.html
                const basePath = currentPath.replace(/_zh\/.*$/, '');
                return basePath + '/';
            } else if (currentPath.includes('_zh')) {
                return currentPath.replace(/_zh/, '');
            }
        }
        
        return currentPath;
    }
    
    // Check if a URL exists (for alternate language version)
    function checkUrlExists(url, callback) {
        // For file:// protocol, we can't reliably check existence
        // So we'll try to load it and handle 404 gracefully
        const xhr = new XMLHttpRequest();
        xhr.open('HEAD', url, true);
        xhr.onreadystatechange = function() {
            if (xhr.readyState === 4) {
                callback(xhr.status === 200);
            }
        };
        xhr.onerror = function() {
            callback(false);
        };
        try {
            xhr.send();
        } catch (e) {
            // If XHR fails (e.g., file:// protocol), assume file exists
            callback(true);
        }
    }
    
    // Create language switcher UI
    function createLanguageSwitcher() {
        console.log('Creating language switcher');
        const currentLang = getCurrentLanguage();
        console.log('Current language:', currentLang);
        
        // Create switcher container
        const switcher = document.createElement('div');
        switcher.id = 'language-switcher';
        switcher.style.cssText = `
            position: fixed;
            top: 10px;
            right: 10px;
            z-index: 9999;
            background: #2980b9;
            padding: 8px 12px;
            border-radius: 4px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.2);
        `;
        
        // Create language links
        const languages = [
            { code: 'en', name: 'English', flag: '🇬🇧' },
            { code: 'zh', name: '中文', flag: '🇨🇳' }
        ];
        
        languages.forEach((lang, index) => {
            const link = document.createElement('a');
            const altUrl = getAlternateUrl(lang.code);
            
            link.href = '#';  // Use # as default to prevent navigation
            link.textContent = `${lang.flag} ${lang.name}`;
            link.style.cssText = `
                color: white;
                text-decoration: none;
                font-size: 14px;
                font-weight: ${currentLang === lang.code ? 'bold' : 'normal'};
                opacity: ${currentLang === lang.code ? '1' : '0.7'};
                margin: 0 5px;
                cursor: pointer;
            `;
            
            // Add hover effect
            link.addEventListener('mouseenter', function() {
                if (currentLang !== lang.code) {
                    this.style.opacity = '1';
                }
            });
            
            link.addEventListener('mouseleave', function() {
                if (currentLang !== lang.code) {
                    this.style.opacity = '0.7';
                }
            });
            
            // Handle click event
            link.addEventListener('click', function(e) {
                e.preventDefault();
                
                // If clicking current language, do nothing
                if (currentLang === lang.code) {
                    return;
                }
                
                // Try to navigate to alternate version
                const targetUrl = altUrl;
                console.log('Attempting to navigate to:', targetUrl);
                
                // Check if alternate version exists
                checkUrlExists(targetUrl, function(exists) {
                    if (exists) {
                        console.log('Alternate version exists, navigating...');
                        window.location.href = targetUrl;
                    } else {
                        console.log('Alternate version does not exist');
                        // Show a friendly message
                        const message = lang.code === 'zh' 
                            ? '抱歉，此页面暂无中文版本。' 
                            : 'Sorry, this page is not available in English.';
                        
                        // Create a temporary notification
                        const notification = document.createElement('div');
                        notification.textContent = message;
                        notification.style.cssText = `
                            position: fixed;
                            top: 50px;
                            right: 10px;
                            background: #e74c3c;
                            color: white;
                            padding: 12px 16px;
                            border-radius: 4px;
                            box-shadow: 0 2px 8px rgba(0,0,0,0.3);
                            z-index: 10000;
                            font-size: 14px;
                            max-width: 300px;
                        `;
                        document.body.appendChild(notification);
                        
                        // Remove notification after 3 seconds
                        setTimeout(function() {
                            notification.style.transition = 'opacity 0.5s';
                            notification.style.opacity = '0';
                            setTimeout(function() {
                                document.body.removeChild(notification);
                            }, 500);
                        }, 3000);
                    }
                });
            });
            
            switcher.appendChild(link);
            
            if (index < languages.length - 1) {
                const separator = document.createElement('span');
                separator.textContent = ' | ';
                separator.style.color = 'white';
                switcher.appendChild(separator);
            }
        });
        
        document.body.appendChild(switcher);
        console.log('Language switcher created and appended to body');
    }
    
    // Initialize when DOM is ready
    function init() {
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', createLanguageSwitcher);
        } else {
            createLanguageSwitcher();
        }
    }
    
    // Try multiple initialization methods to ensure it works
    init();
    
    // Fallback: also try after window load
    window.addEventListener('load', function() {
        if (!document.getElementById('language-switcher')) {
            console.log('Switcher not found, creating again');
            createLanguageSwitcher();
        }
    });
})();
