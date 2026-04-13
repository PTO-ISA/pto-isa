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
 * Navigation collapse control.
 *
 * The ReadTheDocs theme inserts expand buttons and binds its own handlers after
 * theme.js loads. Patch those hooks before the theme enables itself so we can:
 * - keep left-nav scrolling independent from page scrolling
 * - make expand/collapse work consistently for nested sections
 * - keep current-page branches open without forcing the whole tree open
 */
(function () {
    'use strict';

    function directChild(element, selector) {
        return Array.from(element.children).find(function (child) {
            return child.matches(selector);
        }) || null;
    }

    function getItemButton(item) {
        return item.querySelector(':scope > a > button.toctree-expand');
    }

    function getChildMenu(item) {
        return directChild(item, 'ul');
    }

    function getSectionAnchor(item) {
        return directChild(item, 'a.reference.internal');
    }

    function hasChildMenu(item) {
        return !!getChildMenu(item);
    }

    function hydrateSectionLinks() {
        document.querySelectorAll('.wy-menu-vertical li').forEach(function (item) {
            var anchor = getSectionAnchor(item);
            var subMenu = getChildMenu(item);

            if (!anchor || !subMenu || anchor.getAttribute('href')) {
                return;
            }

            var firstChildLink = Array.from(subMenu.querySelectorAll('a.reference.internal[href]')).find(function (link) {
                var href = link.getAttribute('href');
                return href && !href.startsWith('#');
            });

            if (!firstChildLink) {
                return;
            }

            anchor.setAttribute('href', firstChildLink.getAttribute('href'));
            anchor.setAttribute('data-nav-section-link', 'true');
        });
    }

    function setExpanded(item, expanded) {
        var subMenu = getChildMenu(item);
        var button = getItemButton(item);
        if (!subMenu) {
            return;
        }

        subMenu.hidden = !expanded;
        subMenu.style.display = expanded ? 'block' : 'none';
        item.setAttribute('data-nav-expanded', expanded ? 'true' : 'false');
        item.setAttribute('aria-expanded', expanded ? 'true' : 'false');

        if (button) {
            button.setAttribute('type', 'button');
            button.setAttribute('data-expanded', expanded ? 'true' : 'false');
            button.setAttribute('aria-expanded', expanded ? 'true' : 'false');
            button.setAttribute('aria-label', expanded ? 'Collapse section' : 'Expand section');
        }
    }

    function collapseDescendants(item) {
        item.querySelectorAll('li').forEach(function (child) {
            if (hasChildMenu(child)) {
                setExpanded(child, false);
            }
        });
    }

    function scrollCurrentIntoMenu() {
        var menu = document.querySelector('.wy-menu.wy-menu-vertical');
        var currentLink = document.querySelector('.wy-menu-vertical li.current > a.reference.internal[href]');
        if (!menu || !currentLink) {
            return;
        }

        var menuRect = menu.getBoundingClientRect();
        var linkRect = currentLink.getBoundingClientRect();
        var topPadding = 16;
        var bottomPadding = 24;

        if (linkRect.top < menuRect.top + topPadding) {
            menu.scrollTop -= (menuRect.top + topPadding) - linkRect.top;
            return;
        }

        if (linkRect.bottom > menuRect.bottom - bottomPadding) {
            menu.scrollTop += linkRect.bottom - (menuRect.bottom - bottomPadding);
        }
    }

    function syncExpandedState() {
        document.querySelectorAll('.wy-menu-vertical li').forEach(function (item) {
            if (!hasChildMenu(item)) {
                return;
            }

            var expanded = item.classList.contains('current') || item.classList.contains('on');
            setExpanded(item, expanded);
        });

        scrollCurrentIntoMenu();
    }

    function bindButtons() {
        document.querySelectorAll('.wy-menu-vertical button.toctree-expand').forEach(function (button) {
            if (button.hasAttribute('data-pto-bound')) {
                return;
            }

            button.setAttribute('data-pto-bound', 'true');
            button.addEventListener('click', function (event) {
                var item = button.closest('li');
                var expanded;
                if (!item || !hasChildMenu(item)) {
                    return;
                }

                expanded = item.getAttribute('data-nav-expanded') === 'true';
                if (expanded) {
                    collapseDescendants(item);
                }
                setExpanded(item, !expanded);

                event.preventDefault();
                event.stopPropagation();
                event.stopImmediatePropagation();
            }, true);
        });
    }

    function bootstrapTree() {
        hydrateSectionLinks();
        bindButtons();
        syncExpandedState();
    }

    function patchThemeNav() {
        var themeNav = window.SphinxRtdTheme && window.SphinxRtdTheme.Navigation;
        var originalInit;
        var originalReset;

        if (!themeNav || themeNav.__ptoNavPatched) {
            return;
        }

        themeNav.__ptoNavPatched = true;
        originalInit = themeNav.init;
        originalReset = themeNav.reset;

        themeNav.onScroll = function () {
            this.winScroll = false;
        };

        themeNav.init = function (jquery) {
            originalInit.call(this, jquery);
            bootstrapTree();
        };

        themeNav.reset = function () {
            var scrollX = window.scrollX;
            var scrollY = window.scrollY;

            originalReset.call(this);
            window.scrollTo(scrollX, scrollY);
            syncExpandedState();
        };
    }

    patchThemeNav();
    window.addEventListener('load', bootstrapTree, { once: true });

    if (document.readyState !== 'loading') {
        bootstrapTree();
    }
})();
