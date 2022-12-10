import { createElement, Fragment } from 'react'
import { NCMPlugin } from '../loader'

declare global {
    interface Window {
        React: typeof import('react')
        ReactDOM: typeof import('react-dom')
        h: typeof createElement
        f: typeof Fragment

        loadedPlugins: { [pluginId: string]: NCMPlugin }
        loadFailedErrors: [string, Error][]
    }
}
