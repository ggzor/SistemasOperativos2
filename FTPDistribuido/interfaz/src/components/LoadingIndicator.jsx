import * as React from "react"
import styled from "styled-components"
import { IconShell } from "./IconShell"
import { Sync } from "@primer/octicons-react"

export const LoadingIndicator = ({ ...props }) => {
  return <LoadingIndicatorLayout icon={Sync} size={"medium"} {...props} />
}

const LoadingIndicatorLayout = styled(IconShell)`
  color: var(--color-primary);
  transform-origin: center;

  margin-top: 16px;

  display: flex;
  justify-content: stretch;
  align-items: stretch;

  animation: animacion 3s linear infinite;

  @keyframes animacion {
    from {
      transform: rotate(0deg);
    }
    to {
      transform: rotate(360deg);
    }
  }
`
